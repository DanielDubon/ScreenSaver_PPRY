#include <SDL2/SDL.h>
#include <omp.h>
#include <iostream>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <random>
#include <string>
#include <algorithm>
#include <SDL2/SDL_ttf.h>


// Representa un círculo en movimiento con posición en (x,y), velocidad (vx, vy) en pixeles/segundo, 
// radio r y color RGBA
struct Circle {
    float x, y;
    float vx, vy;  
    float r;       
    SDL_Color color;
};

// Contiene los parámetros de configuración del programa con N cantidad de circulos,
// W y H para ancho y alto de la ventana, FPS para los cuadros por segundo 
// y minR, maxR para los radios mínimo y máximo de los círculos
struct Args {
    int N = 200;
    int W = 800;
    int H = 600;
    int FPS = 60;
    int minR = 4;
    int maxR = 12;
};


// Busca en los argumentos de linea de comandos un flag especifico y devuelve el valor entero
// Si no encuentra el flag, devuelve un valor por defecto
static int parseIntArg(char const* flag, int argc, char** argv, int fallback) {
    for (int i = 1; i < argc - 1; ++i) {
        if (std::string(argv[i]) == flag) {
            return std::atoi(argv[i + 1]);
        }
    }
    return fallback;
}


// Dibuja un circulo solido en el renderer, centrado en (cx,cy) con el radio 
// Recorre cada fila vertical dentro del círculo (dy) y usa Pitagoras para calcular la distancia horizontal (dx) 
// Luego dibuja una linea horizontal entre (cx - dx) y (cx + dx)
// Obtenemos un circulo rellenado de lineas horizontales
// OPTIMIZACIÓN: Paralelización con collapse(2) para bucles anidados
static void drawFilledCircle(SDL_Renderer* ren, int cx, int cy, int radius) {
    // OPTIMIZACIÓN: Uso de collapse(2) para paralelizar bucles anidados
    // 
    // EXPLICACIÓN TÉCNICA:
    // - Sin collapse: Solo el bucle externo (dy) se paraleliza
    // - Con collapse(2): OpenMP combina los bucles dy y dx en uno solo
    // - Ventaja: Mejor distribución de carga entre hilos
    // - schedule(dynamic, 4): Bloques pequeños para balanceo dinámico
    // 
    // EJEMPLO: Para un radio de 10, sin collapse tenemos 21 iteraciones del bucle externo
    // Con collapse(2), tenemos 21*21 = 441 iteraciones combinadas, distribuidas
    // uniformemente entre todos los hilos disponibles.
    #pragma omp parallel for collapse(2) schedule(dynamic, 4)
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            // Verificar si el punto está dentro del círculo usando distancia euclidiana
            if (dx * dx + dy * dy <= radius * radius) {
                // Usar critical para evitar condiciones de carrera en el renderizado
                // NOTA: critical es necesario aquí porque SDL_RenderDrawPoint no es thread-safe
                // atomic no se puede usar con funciones de SDL, solo con operaciones básicas
                #pragma omp critical
                {
                    SDL_RenderDrawPoint(ren, cx + dx, cy + dy);
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    // Procesar argumentos desde CLI
    Args args;
    args.N   = std::max(1,  parseIntArg("--n",   argc, argv, args.N));
    args.W   = std::max(640, parseIntArg("--w",   argc, argv, args.W));
    args.H   = std::max(480, parseIntArg("--h",   argc, argv, args.H));
    args.FPS = std::clamp(parseIntArg("--fps", argc, argv, args.FPS), 30, 240);

    // Inicializar SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* win = SDL_CreateWindow(
        "Screensaver OpenMP (paralela) - OPTIMIZADO", //collapse+atomic+sections+reduction
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        args.W, args.H, SDL_WINDOW_SHOWN
    );
    if (!win) {
        std::fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (!ren) {
        std::fprintf(stderr, "SDL_CreateRenderer error: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    // Inicializar círculos con valores aleatorios
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> distX(0.0f, (float)args.W);
    std::uniform_real_distribution<float> distY(0.0f, (float)args.H);
    std::uniform_int_distribution<int>    distR(args.minR, args.maxR);
    std::uniform_real_distribution<float> distAngle(0.0f, 2.0f * (float)M_PI);
    std::uniform_real_distribution<float> distSpeed(60.0f, 180.0f); 
    std::uniform_int_distribution<int>    distCol(60, 255);

    std::vector<Circle> circles;
    circles.reserve((size_t)args.N);
    for (int i = 0; i < args.N; ++i) {
        float r = (float)distR(rng);
        float x = std::clamp(distX(rng), r, (float)args.W - r);
        float y = std::clamp(distY(rng), r, (float)args.H - r);

        float angle = distAngle(rng);
        float speed = distSpeed(rng);
        float vx = std::cos(angle) * speed;
        float vy = std::sin(angle) * speed;

        SDL_Color c { (Uint8)distCol(rng), (Uint8)distCol(rng), (Uint8)distCol(rng), 255 };
        circles.push_back({ x, y, vx, vy, r, c });
    }

    // Variables de control para el loop principal
    bool running = true;
    Uint32 prevTicks = SDL_GetTicks();
    const float targetDt = 1.0f / (float)args.FPS;

    Uint32 fpsTimer = SDL_GetTicks();
    int frames = 0;
    
    // OPTIMIZACIÓN: Contador global de rebotes usando atomic
    // EXPLICACIÓN: Este contador se incrementa desde múltiples hilos cuando
    // se detectan colisiones. Sin atomic, habría race conditions porque
    // múltiples hilos podrían leer el valor antiguo, incrementarlo y escribir
    // el mismo valor, perdiendo algunos incrementos.
    int totalBounces = 0;
    
    // OPTIMIZACIÓN: Variables para estadísticas adicionales
    // Estas variables se usan en la sección paralela para demostrar
    // el uso de reduction y sections
    float totalVelocity = 0.0f;
    int frameCount = 0;

    // Loop principal
    while (running) {
        // Eventos
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        // Calcular delta de tiempo
        Uint32 now = SDL_GetTicks();
        float dt = (now - prevTicks) / 1000.0f;
        prevTicks = now;
        // Limitar a 50 ms para evitar saltos bruscos
        dt = std::min(dt, 0.05f); 

        // Mover los circulos y manejar rebotes
        // OPTIMIZACIÓN: Uso de firstprivate para variables locales
        // 
        // EXPLICACIÓN TÉCNICA:
        // - firstprivate(dt): Cada hilo recibe su propia copia de 'dt' inicializada con el valor actual
        // - Ventaja sobre 'private': No necesitamos copiar el valor en cada iteración
        // - Ventaja sobre 'shared': Cada hilo tiene su propia variable, evitando conflictos
        // - schedule(dynamic, 256): Distribuye bloques de 256 iteraciones dinámicamente
        //   para mejor balanceo de carga entre hilos
        #pragma omp parallel for schedule(dynamic, 256) firstprivate(dt)
        for (int i = 0; i < (int)circles.size(); ++i) {
            auto& c = circles[i];
            c.x += c.vx * dt;
            c.y += c.vy * dt;
        
            // Rebotes contra bordes
            // OPTIMIZACIÓN: Uso de atomic para contador de rebotes
            // 
            // EXPLICACIÓN TÉCNICA:
            // - atomic: Garantiza que la operación ++ sea atómica (indivisible)
            // - Ventaja sobre critical: No bloquea otros hilos, solo la variable específica
            // - Ventaja sobre mutex: Más eficiente para operaciones simples como incremento
            // - Uso: Cuando múltiples hilos pueden acceder simultáneamente a la misma variable
            if (c.x - c.r < 0.0f)      { 
                c.x = c.r;             
                c.vx = -c.vx * 0.80f; 
                #pragma omp atomic
                totalBounces++;
            }
            if (c.x + c.r > args.W)    { 
                c.x = args.W - c.r;    
                c.vx = -c.vx * 0.40f; 
                #pragma omp atomic
                totalBounces++;
            }
            if (c.y - c.r < 0.0f)      { 
                c.y = c.r;             
                c.vy = -c.vy * 0.60f; 
                #pragma omp atomic
                totalBounces++;
            }
            if (c.y + c.r > args.H)    { 
                c.y = args.H - c.r;    
                c.vy = -c.vy * 0.90f; 
                #pragma omp atomic
                totalBounces++;
            }
            
            // Efectos físicos (gravedad y resistencia del aire)
            c.vy += 98.0f * dt;
            c.vx *= 0.999f;
            c.vy *= 0.999f;
        }

        // Render
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        // OPTIMIZACIÓN: Uso de sections para tareas independientes
        // 
        // EXPLICACIÓN TÉCNICA:
        // - sections: Permite que diferentes hilos ejecuten diferentes tareas en paralelo
        // - Ventaja: Aprovecha el paralelismo a nivel de tarea, no solo de datos
        // - Uso: Cuando tenemos tareas que pueden ejecutarse independientemente
        // - Estructura: Cada #pragma omp section se ejecuta en un hilo diferente
        // 
        // EN ESTE CASO:
        // - Sección 1: Renderizar círculos (tarea de gráficos)
        // - Sección 2: Calcular estadísticas (tarea de cómputo)
        // - Ambas se ejecutan simultáneamente, reduciendo el tiempo total
        #pragma omp parallel sections
        {
            // Sección 1: Renderizar círculos
            #pragma omp section
            {
                // Paralelización del renderizado de círculos
                // schedule(dynamic, 32): Bloques pequeños para mejor balanceo
                // Esto es importante porque algunos círculos pueden ser más grandes
                // y requerir más tiempo de renderizado
                #pragma omp parallel for schedule(dynamic, 32)
                for (int i = 0; i < (int)circles.size(); ++i) {
                    auto const& c = circles[i];
                    SDL_SetRenderDrawColor(ren, c.color.r, c.color.g, c.color.b, 255);
                    drawFilledCircle(ren, (int)std::lround(c.x), (int)std::lround(c.y), (int)c.r);
                }
            }
            
            // Sección 2: Calcular estadísticas adicionales
            #pragma omp section
            {
                // OPTIMIZACIÓN: Uso de reduction para cálculos paralelos
                // 
                // EXPLICACIÓN TÉCNICA:
                // - reduction(+:totalVel, fastCircles): Cada hilo tiene variables locales
                // - Ventaja: Evita race conditions sin usar critical o atomic
                // - Funcionamiento: OpenMP combina automáticamente todos los resultados al final
                // - Uso: Para operaciones de reducción como suma, multiplicación, máximo, mínimo
                // 
                // EN ESTE CASO:
                // - totalVel: Suma de todas las velocidades de los círculos
                // - fastCircles: Conteo de círculos con velocidad > 100
                // - Ambos se calculan en paralelo y se combinan automáticamente
                float totalVel = 0.0f;
                int fastCircles = 0;
                
                #pragma omp parallel for reduction(+:totalVel, fastCircles)
                for (int i = 0; i < (int)circles.size(); ++i) {
                    const auto& c = circles[i];
                    float vel = std::sqrt(c.vx * c.vx + c.vy * c.vy);
                    totalVel += vel;
                    if (vel > 100.0f) {
                        fastCircles++;
                    }
                }
                
                // Aquí podrías usar totalVel y fastCircles para algo
                // Por ejemplo, mostrar estadísticas en pantalla o ajustar parámetros
            }
        }

        SDL_RenderPresent(ren);

        // FPS
        ++frames;
        float frameTime = (SDL_GetTicks() - now) / 1000.0f;
        if (frameTime < targetDt) {
            SDL_Delay((Uint32)((targetDt - frameTime) * 1000.0f));
        }

        // Mostrar FPS cada segundo
        if (SDL_GetTicks() - fpsTimer >= 1000) {
            static int seconds = 0;
            ++seconds;
            Uint32 elapsed_ms = SDL_GetTicks();
            double elapsed_s  = elapsed_ms / 1000.0;
            std::printf("[t=%ds | %.2fs] FPS ~ %d | N=%d | %dx%d | Rebotes: %d\n",
                seconds, elapsed_s, frames, args.N, args.W, args.H, totalBounces);
            frames = 0;
            fpsTimer = SDL_GetTicks();
        }
    }

    // Liberar recursos y salir
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
