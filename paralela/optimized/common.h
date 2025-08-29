/*
 * HEADER COMÚN - Estructuras compartidas
 */

#ifndef COMMON_H
#define COMMON_H

#include <SDL2/SDL.h>

// Representa un círculo en movimiento
struct Circle {
    float x, y;
    float vx, vy;  
    float r;       
    SDL_Color color;
};

#endif
