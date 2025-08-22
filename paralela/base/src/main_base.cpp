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
static void drawFilledCircle(SDL_Renderer* ren, int cx, int cy, int radius) {
    for (int dy = -radius; dy <= radius; ++dy) {
        int dx = (int)std::sqrt((double)(radius * radius - dy * dy));
        SDL_RenderDrawLine(ren, cx - dx, cy + dy, cx + dx, cy + dy);
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
        "Screensaver OpenMP (paralela)",
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
        #pragma omp parallel for schedule(dynamic, 256)
        for (int i = 0; i < (int)circles.size(); ++i) {
            auto& c = circles[i];
            c.x += c.vx * dt;
            c.y += c.vy * dt;
        
            // Rebotes contra bordes
            if (c.x - c.r < 0.0f)      { c.x = c.r;             c.vx = -c.vx * 0.80f; }
            if (c.x + c.r > args.W)    { c.x = args.W - c.r;    c.vx = -c.vx * 0.40f; }
            if (c.y - c.r < 0.0f)      { c.y = c.r;             c.vy = -c.vy * 0.60f; }
            if (c.y + c.r > args.H)    { c.y = args.H - c.r;    c.vy = -c.vy * 0.90f; }
        }

        // Render
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        for (auto const& c : circles) {
            SDL_SetRenderDrawColor(ren, c.color.r, c.color.g, c.color.b, 255);
            drawFilledCircle(ren, (int)std::lround(c.x), (int)std::lround(c.y), (int)c.r);
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
            std::printf("[t=%ds | %.2fs] FPS ~ %d | N=%d | %dx%d\n",
                seconds, elapsed_s, frames, args.N, args.W, args.H);
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
