#pragma once
#include <SDL2/SDL.h>
#include <cstdint>
#include <vector>
#include "sim.hpp"

// Framebuffer en RAM que representa una imagen completa en memoria con formato ABGR8888
// Se usa para dibujar los circulos con OpenMP en paralelo y luego se pasa como textura a SDL en el hilo principal
struct Framebuffer {
    int W, H;
    std::vector<uint32_t> pix; // vector de pixeles en formato ABGR

    explicit Framebuffer(int w, int h)
        : W(w), H(h), pix((size_t)w * h, 0) {}

    // Limpia el framebuffer llenándolo con un color sólido
    void clear(uint32_t abgr) {
        std::fill(pix.begin(), pix.end(), abgr);
    }
};


// Funciones de renderizado

// Dibuja todos los circulos en tiles 2D dentro del framebuffer
void render_circles_tiled_omp(const ParticlesSOA& P, Framebuffer& fb);

// Convierte el framebuffer a textura SDL y lo presenta en pantalla
void present_framebuffer(SDL_Renderer* ren, const Framebuffer& fb);
