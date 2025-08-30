/*
 * SIMULACIÓN OPTIMIZADA - OpenMP Avanzado
 * 
 * Este header contiene las funciones de simulación usando
 * cláusulas OpenMP avanzadas.
 */

#ifndef SIMULATION_OPTIMIZED_H
#define SIMULATION_OPTIMIZED_H

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
#include <chrono>
#include "common.h"

// Función de cómputo pesado CON optimizaciones OpenMP avanzadas
// Implementa cláusulas OpenMP avanzadas
double heavyComputationOptimized(int iterations) {
    double result = 0.0;
    
    // OPTIMIZACIÓN: Usar collapse(2) solo para bucles grandes
    // Para bucles pequeños, el overhead de collapse puede ser mayor que el beneficio
    if (iterations > 50) {
        // Para bucles grandes, usar collapse(2) es más eficiente
        #pragma omp parallel for collapse(2) schedule(dynamic, 25) reduction(+:result)
        for (int i = 0; i < iterations; ++i) {
            for (int j = 0; j < 50; ++j) {
                // Cálculo matemático intensivo optimizado
                result += std::sin(i * 0.01) * std::cos(j * 0.02) * std::tan((i + j) * 0.03);
            }
        }
    } else {
        // Para bucles pequeños, usar paralelización simple es más eficiente
        #pragma omp parallel for schedule(dynamic, 10) reduction(+:result)
        for (int i = 0; i < iterations; ++i) {
            for (int j = 0; j < 50; ++j) {
                result += std::sin(i * 0.01) * std::cos(j * 0.02) * std::tan((i + j) * 0.03);
            }
        }
    }
    return result;
}

// Simulación de física CON optimizaciones OpenMP avanzadas
// Implementa cláusulas OpenMP avanzadas
void runSimulationOptimized(std::vector<Circle>& circles, int& totalBounces, double& totalEnergy, int& iterations) {
    iterations = 0;
    totalBounces = 0;
    totalEnergy = 0.0;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(10); // Ejecutar por exactamente 10 segundos
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // ACTUALIZACIÓN DE FÍSICA con optimizaciones OpenMP avanzadas
        
        // OPTIMIZACIÓN: Usar firstprivate para dt (más eficiente que private)
        float dt = 0.016f;
        
        // OPTIMIZACIÓN: Ajustar el tamaño del chunk según el número de círculos
        int chunk_size = std::max(1, (int)circles.size() / (omp_get_max_threads() * 4));
        #pragma omp parallel for schedule(dynamic, chunk_size) firstprivate(dt)
        for (int i = 0; i < (int)circles.size(); ++i) {
            auto& c = circles[i];
            
            // Actualización de posición (cada hilo procesa diferentes círculos)
            c.x += c.vx * dt;
            c.y += c.vy * dt;
            
            // OPTIMIZACIÓN: Usar atomic para contadores (más eficiente que critical)
            // atomic: Garantiza que el incremento de totalBounces sea thread-safe
            if (c.x - c.r < 0.0f) {
                c.x = c.r;
                c.vx = -c.vx * 0.80f;
                #pragma omp atomic
                totalBounces++;
            }
            if (c.x + c.r > 800.0f) {
                c.x = 800.0f - c.r;
                c.vx = -c.vx * 0.80f;
                #pragma omp atomic
                totalBounces++;
            }
            if (c.y - c.r < 0.0f) {
                c.y = c.r;
                c.vy = -c.vy * 0.80f;
                #pragma omp atomic
                totalBounces++;
            }
            if (c.y + c.r > 600.0f) {
                c.y = 600.0f - c.r;
                c.vy = -c.vy * 0.80f;
                #pragma omp atomic
                totalBounces++;
            }
            
            // Efectos físicos (gravedad y resistencia del aire)
            c.vy += 98.0f * dt;
            c.vx *= 0.999f;
            c.vy *= 0.999f;
        }
        
        // OPTIMIZACIÓN: Cálculo de energía con reducción paralela
        // reduction(+:kinetic, potential): Cada hilo suma en variables locales
        double kinetic = 0.0, potential = 0.0;
        #pragma omp parallel for reduction(+:kinetic, potential) schedule(dynamic, chunk_size)
        for (int i = 0; i < (int)circles.size(); ++i) {
            const auto& c = circles[i];
            float vel = std::sqrt(c.vx * c.vx + c.vy * c.vy);
            kinetic += 0.5 * c.r * c.r * vel * vel;  // Energía cinética
            potential += 9.8 * c.r * c.r * c.y;      // Energía potencial
        }
        totalEnergy = kinetic + potential;
        
        // OPTIMIZACIÓN: Cómputo pesado adaptativo - REDUCIDO para mejor speedup
        // Ajustar la intensidad según el número de círculos, pero más conservador
        int compute_intensity = std::min(100, std::max(20, (int)circles.size() / 20));
        heavyComputationOptimized(compute_intensity);
        
        iterations++;
    }
}

#endif
