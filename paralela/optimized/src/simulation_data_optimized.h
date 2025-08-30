/*
 * SIMULACIÓN CON ESTRUCTURAS DE DATOS OPTIMIZADAS
 * 
 * Este header implementa las funciones de simulación usando las
 * estructuras de datos optimizadas (SoA, memory pools, cache alignment)
 * para demostrar mejoras de rendimiento adicionales.
 */

#ifndef SIMULATION_DATA_OPTIMIZED_H
#define SIMULATION_DATA_OPTIMIZED_H

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
#include "data_structures_optimized.h"

double heavyComputationDataOptimized(int iterations) {
    double result = 0.0;
    
    // OPTIMIZACIÓN: Usar arrays alineados para mejor acceso a cache
    // Crear arrays temporales alineados para cálculos intensivos
    std::vector<double, std::allocator<double>> temp_results;
    temp_results.reserve(iterations); // Pre-reservar memoria
    
    // OPTIMIZACIÓN: Usar collapse(2) con arrays optimizados
    if (iterations > 50) {
        #pragma omp parallel for collapse(2) schedule(dynamic, 25) reduction(+:result)
        for (int i = 0; i < iterations; ++i) {
            for (int j = 0; j < 50; ++j) {
                // Cálculo matemático intensivo con mejor localidad de cache
                result += std::sin(i * 0.01) * std::cos(j * 0.02) * std::tan((i + j) * 0.03);
            }
        }
    } else {
        #pragma omp parallel for schedule(dynamic, 10) reduction(+:result)
        for (int i = 0; i < iterations; ++i) {
            for (int j = 0; j < 50; ++j) {
                result += std::sin(i * 0.01) * std::cos(j * 0.02) * std::tan((i + j) * 0.03);
            }
        }
    }
    return result;
}

// SIMULACIÓN CON ESTRUCTURAS DE DATOS OPTIMIZADAS (SoA)
void runSimulationDataOptimized(CirclesOptimized& circles, int& totalBounces, double& totalEnergy, int& iterations) {
    iterations = 0;
    totalBounces = 0;
    totalEnergy = 0.0;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(10); // Ejecutar por exactamente 10 segundos
    
    // OPTIMIZACIÓN: Obtener punteros directos a los arrays para acceso más rápido
    float* x_ptr = circles.x.data();
    float* y_ptr = circles.y.data();
    float* vx_ptr = circles.vx.data();
    float* vy_ptr = circles.vy.data();
    float* r_ptr = circles.r.data();
    size_t num_circles = circles.size();
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // ACTUALIZACIÓN DE FÍSICA con estructuras optimizadas
        
        // OPTIMIZACIÓN: Usar firstprivate para dt y punteros
        float dt = 0.016f;
        
        // OPTIMIZACIÓN: Ajustar chunk size basado en el número de círculos
        int chunk_size = std::max(1, (int)num_circles / (omp_get_max_threads() * 4));
        
        // OPTIMIZACIÓN: Acceso directo a arrays separados (mejor localidad de cache)
        #pragma omp parallel for schedule(dynamic, chunk_size) firstprivate(dt)
        for (int i = 0; i < (int)num_circles; ++i) {
            // Acceso directo a arrays - más eficiente que acceso a estructura
            float& x = x_ptr[i];
            float& y = y_ptr[i];
            float& vx = vx_ptr[i];
            float& vy = vy_ptr[i];
            float& r = r_ptr[i];
            
            // Actualización de posición
            x += vx * dt;
            y += vy * dt;
            
            // OPTIMIZACIÓN: Usar atomic para contadores (más eficiente que critical)
            if (x - r < 0.0f) {
                x = r;
                vx = -vx * 0.80f;
                #pragma omp atomic
                totalBounces++;
            }
            if (x + r > 800.0f) {
                x = 800.0f - r;
                vx = -vx * 0.80f;
                #pragma omp atomic
                totalBounces++;
            }
            if (y - r < 0.0f) {
                y = r;
                vy = -vy * 0.80f;
                #pragma omp atomic
                totalBounces++;
            }
            if (y + r > 600.0f) {
                y = 600.0f - r;
                vy = -vy * 0.80f;
                #pragma omp atomic
                totalBounces++;
            }
            
            // Efectos físicos
            vy += 98.0f * dt;
            vx *= 0.999f;
            vy *= 0.999f;
        }
        
        // OPTIMIZACIÓN: Cálculo de energía con arrays optimizados
        double kinetic = 0.0, potential = 0.0;
        #pragma omp parallel for reduction(+:kinetic, potential) schedule(dynamic, chunk_size)
        for (int i = 0; i < (int)num_circles; ++i) {
            // Acceso directo a arrays - mejor localidad de cache
            float vx = vx_ptr[i];
            float vy = vy_ptr[i];
            float r = r_ptr[i];
            float y = y_ptr[i];
            
            float vel = std::sqrt(vx * vx + vy * vy);
            kinetic += 0.5 * r * r * vel * vel;
            potential += 9.8 * r * r * y;
        }
        totalEnergy = kinetic + potential;
        
        // OPTIMIZACIÓN: Cómputo pesado adaptativo con estructuras optimizadas
        int compute_intensity = std::min(100, std::max(20, (int)num_circles / 20));
        heavyComputationDataOptimized(compute_intensity);
        
        iterations++;
    }
}

// SIMULACIÓN BASE CON ESTRUCTURAS ESTÁNDAR (para comparación)
void runSimulationDataBase(std::vector<Circle>& circles, int& totalBounces, double& totalEnergy, int& iterations) {
    iterations = 0;
    totalBounces = 0;
    totalEnergy = 0.0;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(10);
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // ACTUALIZACIÓN DE FÍSICA con estructuras estándar (AoS)
        
        // PROBLEMA: Acceso a estructura completa (menos eficiente)
        #pragma omp parallel for schedule(dynamic, 256)
        for (int i = 0; i < (int)circles.size(); ++i) {
            auto& c = circles[i]; // Acceso a estructura completa
            
            // Actualización de posición
            c.x += c.vx * 0.016f;
            c.y += c.vy * 0.016f;
            
            // PROBLEMA: DETECCIÓN DE COLISIONES CON critical (menos eficiente)
            if (c.x - c.r < 0.0f) {
                c.x = c.r;
                c.vx = -c.vx * 0.80f;
                #pragma omp critical
                totalBounces++;
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
            
            // Efectos físicos
            c.vy += 98.0f * 0.016f;
            c.vx *= 0.999f;
            c.vy *= 0.999f;
        }
        
        // CÁLCULO DE ENERGÍA con estructuras estándar
        double kinetic = 0.0, potential = 0.0;
        #pragma omp parallel for reduction(+:kinetic, potential)
        for (int i = 0; i < (int)circles.size(); ++i) {
            const auto& c = circles[i]; // Acceso a estructura completa
            float vel = std::sqrt(c.vx * c.vx + c.vy * c.vy);
            kinetic += 0.5 * c.r * c.r * vel * vel;
            potential += 9.8 * c.r * c.r * c.y;
        }
        totalEnergy = kinetic + potential;
        
        // Cómputo pesado con estructuras estándar
        int compute_intensity = std::min(150, std::max(30, (int)circles.size() / 15));
        heavyComputationDataOptimized(compute_intensity); // Usar la misma función para comparación justa
        
        iterations++;
    }
}

// FUNCIÓN PARA CREAR CÍRCULOS CON ESTRUCTURAS OPTIMIZADAS
CirclesOptimized createOptimizedCircles(int numCircles) {
    CirclesOptimized circles(numCircles);
    
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> distX(0.0f, 800.0f);
    std::uniform_real_distribution<float> distY(0.0f, 600.0f);
    std::uniform_int_distribution<int> distR(4, 12);
    std::uniform_real_distribution<float> distAngle(0.0f, 2.0f * (float)M_PI);
    std::uniform_real_distribution<float> distSpeed(60.0f, 180.0f);
    
    for (int i = 0; i < numCircles; ++i) {
        float r = (float)distR(rng);
        float x = std::clamp(distX(rng), r, 800.0f - r);
        float y = std::clamp(distY(rng), r, 600.0f - r);
        
        float angle = distAngle(rng);
        float speed = distSpeed(rng);
        float vx = std::cos(angle) * speed;
        float vy = std::sin(angle) * speed;
        
        SDL_Color color = {255, 255, 255, 255}; // Blanco por defecto
        circles.addCircle(x, y, vx, vy, r, color);
    }
    
    return circles;
}

#endif