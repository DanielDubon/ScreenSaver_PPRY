/*
 * SIMULACIÓN BASE - OpenMP Básico
 * 
 * Este header contiene las funciones de simulación usando solo
 * cláusulas OpenMP básicas
 */

#ifndef SIMULATION_BASE_H
#define SIMULATION_BASE_H

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

// Función de cómputo pesado SIN optimizaciones OpenMP avanzadas
// Usa SOLO OpenMP básico
double heavyComputationBase(int iterations) {
    double result = 0.0;
    
    // SOLO paralelización del bucle externo básico
    // reduction(+:result): Cada hilo tiene su propia copia de 'result' y al final se suman
    // schedule(dynamic, 100): Distribuye bloques de 100 iteraciones dinámicamente
    #pragma omp parallel for reduction(+:result) schedule(dynamic, 100)
    for (int i = 0; i < iterations; ++i) {
        double temp = 0.0;
        
        // PROBLEMA: Los bucles anidados NO están paralelizados
        // Solo el bucle externo se ejecuta en paralelo, los internos son secuenciales
        // Esto crea un desbalance de carga porque algunos hilos tienen más trabajo
        for (int j = 0; j < 100; ++j) {
            for (int k = 0; k < 100; ++k) {
                temp += std::sin(i * 0.01) * std::cos(j * 0.02) * std::tan(k * 0.03);
            }
        }
        result += temp;
    }
    return result;
}

// Simulación de física SIN optimizaciones OpenMP avanzadas
// Usa SOLO OpenMP básico 
void runSimulationBase(std::vector<Circle>& circles, int& totalBounces, double& totalEnergy, int& iterations) {
    iterations = 0;
    totalBounces = 0;
    totalEnergy = 0.0;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(10); // Ejecutar por exactamente 10 segundos
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // SOLO paralelización del bucle principal
        // schedule(dynamic, 256): Distribuye bloques de 256 iteraciones dinámicamente
        #pragma omp parallel for schedule(dynamic, 256)
        for (int i = 0; i < (int)circles.size(); ++i) {
            auto& c = circles[i];
            
            // Actualización de posición (cada hilo procesa diferentes círculos)
            c.x += c.vx * 0.016f;
            c.y += c.vy * 0.016f;
            
            // PROBLEMA: DETECCIÓN DE COLISIONES CON critical (menos eficiente)
            // critical: Bloquea TODOS los hilos cuando uno accede a totalBounces
            // Esto crea un cuello de botella porque los hilos esperan entre sí
            if (c.x - c.r < 0.0f) {
                c.x = c.r;
                c.vx = -c.vx * 0.80f;
                #pragma omp critical
                totalBounces++;  // Bloquea todos los hilos
            }
            if (c.x + c.r > 800.0f) {
                c.x = 800.0f - c.r;
                c.vx = -c.vx * 0.80f;
                #pragma omp critical
                totalBounces++;
            }
            if (c.y - c.r < 0.0f) {
                c.y = c.r;
                c.vy = -c.vy * 0.80f;
                #pragma omp critical
                totalBounces++;
            }
            if (c.y + c.r > 600.0f) {
                c.y = 600.0f - c.r;
                c.vy = -c.vy * 0.80f;
                #pragma omp critical
                totalBounces++;
            }
            
            // Efectos físicos (gravedad y resistencia del aire)
            c.vy += 98.0f * 0.016f;
            c.vx *= 0.999f;
            c.vy *= 0.999f;
        }
        
        // CÁLCULO DE ENERGÍA usando reducción básica
        // reduction(+:kinetic, potential): Cada hilo suma en variables locales
        double kinetic = 0.0, potential = 0.0;
        #pragma omp parallel for reduction(+:kinetic, potential)
        for (int i = 0; i < (int)circles.size(); ++i) {
            const auto& c = circles[i];
            float vel = std::sqrt(c.vx * c.vx + c.vy * c.vy);
            kinetic += 0.5 * c.r * c.r * vel * vel;  // Energía cinética
            potential += 9.8 * c.r * c.r * c.y;      // Energía potencial
        }
        totalEnergy = kinetic + potential;
        
        // Cómputo pesado SIN optimizaciones avanzadas - MÁS INTENSIVO para comparación
        // Aumentar la intensidad para hacer la versión base más lenta
        int compute_intensity = std::min(150, std::max(30, (int)circles.size() / 15));
        heavyComputationBase(compute_intensity);
        
        iterations++;
    }
}

#endif
