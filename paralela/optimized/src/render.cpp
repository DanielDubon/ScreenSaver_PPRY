#include "render.hpp"
#include <algorithm>
#include <cmath>
#ifdef _OPENMP
  #include <omp.h>
#endif

// Escribe un pixel de color 'c' en (x,y) dentro del framebuffer
// Se valida que (x,y) esté dentro de los limites de la imagen
static inline void putpix(Framebuffer& fb, int x, int y, uint32_t c){
    if ((unsigned)x >= (unsigned)fb.W || (unsigned)y >= (unsigned)fb.H) return;
    fb.pix[(size_t)y * fb.W + x] = c;
}

// Dibuja un circulo en el framebuffer, pero recortado a los limites de un tile rectangular [x0,x1) × [y0,y1)
// Usa scanlines horizontales para rellenar el circulo
static inline void draw_circle_clipped_to_tile(
    Framebuffer& fb, int cx, int cy, int r, uint32_t color,
    int x0, int y0, int x1, int y1
){
    int yy_from = std::max(cy - r, y0);
    int yy_to   = std::min(cy + r, y1 - 1);
    for (int yy = yy_from; yy <= yy_to; ++yy){
        int dy = yy - cy;
        int dx = (int)std::floor(std::sqrt((double)r * r - (double)dy * dy));
        int xx0 = std::max(cx - dx, x0);
        int xx1 = std::min(cx + dx, x1 - 1);
        size_t row = (size_t)yy * fb.W;
        for (int xx = xx0; xx <= xx1; ++xx){
            fb.pix[row + xx] = color;
        }
    }
}

// Divide la pantalla en tiles de tamaño fijo y signa a cada tile la lista de particulas que lo pueden cubrir (binning)
// Tambien procesa cada tile en paralelo con OpenMP taskloop, pintando solo su region
void render_circles_tiled_omp(const ParticlesSOA& P, Framebuffer& fb){
    constexpr int TILE = 32;
    const int tiles_x = (fb.W + TILE - 1) / TILE;
    const int tiles_y = (fb.H + TILE - 1) / TILE;
    const int total_tiles = tiles_x * tiles_y;

    // Binning de particulas a tiles
    std::vector<std::vector<int>> bins((size_t)total_tiles);
    bins.shrink_to_fit();

    for (int i = 0; i < P.n; ++i){
        const int cx = (int)std::lround(P.x[i]);
        const int cy = (int)std::lround(P.y[i]);
        const int r  = P.r[i];

        int min_tx = std::max(0, (cx - r) / TILE);
        int max_tx = std::min(tiles_x - 1, (cx + r) / TILE);
        int min_ty = std::max(0, (cy - r) / TILE);
        int max_ty = std::min(tiles_y - 1, (cy + r) / TILE);

        for (int ty = min_ty; ty <= max_ty; ++ty){
            for (int tx = min_tx; tx <= max_tx; ++tx){
                bins[(size_t)ty * tiles_x + tx].push_back(i);
            }
        }
    }

    // Procesa cada tile en paralelo
#ifdef _OPENMP
    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            #pragma omp taskloop grainsize(1)
            for (int tid = 0; tid < total_tiles; ++tid){
                int tx = tid % tiles_x;
                int ty = tid / tiles_x;

                int x0 = tx * TILE;
                int x1 = std::min(x0 + TILE, fb.W);
                int y0 = ty * TILE;
                int y1 = std::min(y0 + TILE, fb.H);

                const auto& vec = bins[(size_t)tid];
                for (int idx : vec){
                    draw_circle_clipped_to_tile(
                        fb,
                        (int)std::lround(P.x[idx]),
                        (int)std::lround(P.y[idx]),
                        P.r[idx],
                        P.color[idx],
                        x0, y0, x1, y1
                    );
                }
            }
        }
    }
#else
    // Fallback secuencial (sin OpenMP)
    for (int tid = 0; tid < total_tiles; ++tid){
        int tx = tid % tiles_x;
        int ty = tid / tiles_x;
        int x0 = tx * TILE;
        int x1 = std::min(x0 + TILE, fb.W);
        int y0 = ty * TILE;
        int y1 = std::min(y0 + TILE, fb.H);

        const auto& vec = bins[(size_t)tid];
        for (int idx : vec){
            draw_circle_clipped_to_tile(
                fb,
                (int)std::lround(P.x[idx]),
                (int)std::lround(P.y[idx]),
                P.r[idx],
                P.color[idx],
                x0, y0, x1, y1
            );
        }
    }
#endif
}


// Convierte el framebuffer en una textura SDL y la muestra
void present_framebuffer(SDL_Renderer* ren, const Framebuffer& fb){
    SDL_Texture* tex = SDL_CreateTexture(
        ren, SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING, fb.W, fb.H
    );
    SDL_UpdateTexture(tex, nullptr, fb.pix.data(), fb.W * (int)sizeof(uint32_t));
    SDL_RenderClear(ren);
    SDL_RenderCopy(ren, tex, nullptr, nullptr);
    SDL_RenderPresent(ren);
    SDL_DestroyTexture(tex);
}
