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
#include "common.h"

struct Args {
    int N = 500;
    int W = 1200;
    int H = 800;
    int FPS = 60;
    int minR = 3;
    int maxR = 8;
    int num_threads = omp_get_max_threads();
};

static int parseIntArg(char const* flag, int argc, char** argv, int fallback) {
    for (int i = 1; i < argc - 1; ++i) {
        if (std::string(argv[i]) == flag) {
            return std::atoi(argv[i + 1]);
        }
    }
    return fallback;
}

static void drawFilledCircle(SDL_Renderer* ren, int cx, int cy, int radius) {
    for (int dy = -radius; dy <= radius; ++dy) {
        int dx = (int)std::sqrt((double)(radius * radius - dy * dy));
        SDL_RenderDrawLine(ren, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

int main(int argc, char** argv) {
    Args args;
    args.N = std::max(1, parseIntArg("--n", argc, argv, args.N));
    args.W = std::max(640, parseIntArg("--w", argc, argv, args.W));
    args.H = std::max(480, parseIntArg("--h", argc, argv, args.H));
    args.FPS = std::clamp(parseIntArg("--fps", argc, argv, args.FPS), 30, 240);
    args.num_threads = std::max(1, parseIntArg("--threads", argc, argv, args.num_threads));

    omp_set_num_threads(args.num_threads);

    std::printf("=== DEMOSTRACIÓN VISUAL PARALELO OPTIMIZADO ===\n");
    std::printf("Círculos: %d | Resolución: %dx%d | Hilos: %d\n", 
                args.N, args.W, args.H, args.num_threads);
    std::printf("Optimizaciones aplicadas: OpenMP avanzado + Estructuras + Memoria + SIMD\n");
    std::printf("Presiona ESC para salir\n\n");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* win = SDL_CreateWindow(
        "Screensaver Paralelo Optimizado - Demo Visual",
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

    // OPTIMIZACIÓN B: Pre-asignar memoria exacta
    std::vector<Circle> circles;
    circles.reserve((size_t)args.N);

    // Inicialización con mismos parámetros que versión optimizada
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> distX(0.0f, (float)args.W);
    std::uniform_real_distribution<float> distY(0.0f, (float)args.H);
    std::uniform_int_distribution<int> distR(args.minR, args.maxR);
    std::uniform_real_distribution<float> distAngle(0.0f, 2.0f * (float)M_PI);
    std::uniform_real_distribution<float> distSpeed(60.0f, 180.0f);
    std::uniform_int_distribution<int> distCol(100, 255);

    for (int i = 0; i < args.N; ++i) {
        float r = (float)distR(rng);
        float x = std::clamp(distX(rng), r, (float)args.W - r);
        float y = std::clamp(distY(rng), r, (float)args.H - r);

        float angle = distAngle(rng);
        float speed = distSpeed(rng);
        float vx = std::cos(angle) * speed;
        float vy = std::sin(angle) * speed;

        SDL_Color c{(Uint8)distCol(rng), (Uint8)distCol(rng), (Uint8)distCol(rng), 255};
        circles.push_back({x, y, vx, vy, r, c});
    }

    bool running = true;
    Uint32 prevTicks = SDL_GetTicks();
    const float targetDt = 1.0f / (float)args.FPS;

    Uint32 fpsTimer = SDL_GetTicks();
    int frames = 0;
    int bounces = 0;

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        Uint32 now = SDL_GetTicks();
        float dt = (now - prevTicks) / 1000.0f;
        prevTicks = now;
        dt = std::min(dt, 0.05f);

        // SIMULACIÓN PARALELA OPTIMIZADA
        // OPTIMIZACIÓN A: Cláusulas OpenMP avanzadas con scheduling estático
        // OPTIMIZACIÓN B: Estructuras pre-asignadas
        // OPTIMIZACIÓN C: Variables locales para reducir accesos a memoria
        // OPTIMIZACIÓN D: Código SIMD-friendly
        #pragma omp parallel for schedule(static, 32) reduction(+:bounces) \
                 num_threads(args.num_threads)
        for (size_t i = 0; i < circles.size(); ++i) {
            auto& circle = circles[i];
            
            // OPTIMIZACIÓN C: Variables locales
            float x = circle.x;
            float y = circle.y;
            float vx = circle.vx;
            float vy = circle.vy;
            const float r = circle.r;
            
            // OPTIMIZACIÓN D: Operaciones vectorizables
            x += vx * dt;
            y += vy * dt;
            
            // OPTIMIZACIÓN C: Detección de colisiones optimizada
            if (x - r <= 0) {
                vx = -vx * 0.95f; // Pequeña pérdida de energía
                x = r;
                bounces++;
            } else if (x + r >= args.W) {
                vx = -vx * 0.95f;
                x = args.W - r;
                bounces++;
            }
            
            if (y - r <= 0) {
                vy = -vy * 0.95f;
                y = r;
                bounces++;
            } else if (y + r >= args.H) {
                vy = -vy * 0.95f;
                y = args.H - r;
                bounces++;
            }
            
            // OPTIMIZACIÓN C: Escribir de vuelta solo al final
            circle.x = x;
            circle.y = y;
            circle.vx = vx;
            circle.vy = vy;
        }

        // Renderizado (secuencial)
        SDL_SetRenderDrawColor(ren, 15, 15, 30, 255);
        SDL_RenderClear(ren);

        for (const auto& c : circles) {
            SDL_SetRenderDrawColor(ren, c.color.r, c.color.g, c.color.b, 255);
            drawFilledCircle(ren, (int)std::lround(c.x), (int)std::lround(c.y), (int)c.r);
        }

        SDL_RenderPresent(ren);

        ++frames;
        float frameTime = (SDL_GetTicks() - now) / 1000.0f;
        if (frameTime < targetDt) {
            SDL_Delay((Uint32)((targetDt - frameTime) * 1000.0f));
        }

        if (SDL_GetTicks() - fpsTimer >= 1000) {
            static int seconds = 0;
            ++seconds;
            double avg_bounces_per_second = bounces / (double)seconds;
            
            std::printf("[%ds] FPS: %d | Círculos: %d | Hilos: %d | Rebotes/s: %.0f | Total: %d\n",
                       seconds, frames, args.N, args.num_threads, avg_bounces_per_second, bounces);
            
            frames = 0;
            fpsTimer = SDL_GetTicks();
        }
    }

    std::printf("\n=== DEMOSTRACIÓN COMPLETADA ===\n");
    std::printf("Total de rebotes: %d\n", bounces);
    std::printf("Optimizaciones aplicadas exitosamente\n");

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}