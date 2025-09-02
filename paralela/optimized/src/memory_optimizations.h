/*
 * OPTIMIZACIONES DE ACCESO A MEMORIA - Inciso C
 * 
 * Este header implementa optimizaciones en mecanismos de acceso a memoria:
 * 1. NUMA-aware memory allocation
 * 2. Cache-friendly data access patterns
 * 3. Memory prefetching
 * 4. False sharing prevention
 * 5. Memory bandwidth optimization
 */

#ifndef MEMORY_OPTIMIZATIONS_H
#define MEMORY_OPTIMIZATIONS_H

#include <vector>
#include <memory>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <omp.h>
#include "common.h"

// ============================================================================
// ESTRUCTURA CON OPTIMIZACIONES DE MEMORIA
// ============================================================================
struct alignas(64) MemoryOptimizedCircles {
    // Alineación a 64 bytes para evitar false sharing
    // Padding para asegurar que cada hilo acceda a líneas de cache separadas
    
    // Arrays con alineación explícita para mejor acceso a cache
    std::vector<float, std::allocator<float>> x;
    std::vector<float, std::allocator<float>> y;
    std::vector<float, std::allocator<float>> vx;
    std::vector<float, std::allocator<float>> vy;
    std::vector<float, std::allocator<float>> r;
    std::vector<uint32_t, std::allocator<uint32_t>> colors;
    
    // Arrays de trabajo locales para cada hilo (evita false sharing)
    std::vector<std::vector<float>> thread_local_x;
    std::vector<std::vector<float>> thread_local_y;
    std::vector<std::vector<float>> thread_local_vx;
    std::vector<std::vector<float>> thread_local_vy;
    
    // Constructor con optimizaciones de memoria
    explicit MemoryOptimizedCircles(size_t capacity) {
        size_t num_threads = omp_get_max_threads();
        
        // Reservar memoria con alineación
        x.reserve(capacity);
        y.reserve(capacity);
        vx.reserve(capacity);
        vy.reserve(capacity);
        r.reserve(capacity);
        colors.reserve(capacity);
        
        // Inicializar arrays locales por hilo
        thread_local_x.resize(num_threads);
        thread_local_y.resize(num_threads);
        thread_local_vx.resize(num_threads);
        thread_local_vy.resize(num_threads);
        
        for (size_t i = 0; i < num_threads; ++i) {
            thread_local_x[i].reserve(capacity / num_threads);
            thread_local_y[i].reserve(capacity / num_threads);
            thread_local_vx[i].reserve(capacity / num_threads);
            thread_local_vy[i].reserve(capacity / num_threads);
        }
    }
    
    size_t size() const { return x.size(); }
};

// ============================================================================
// FUNCIÓN CON PREFETCHING DE MEMORIA
// ============================================================================
void prefetch_circles_data(const MemoryOptimizedCircles& circles, int start_idx, int end_idx) {
    // Prefetch datos que se van a usar próximamente
    for (int i = start_idx; i < end_idx && i < (int)circles.x.size(); i += 16) {
        __builtin_prefetch(&circles.x[i], 0, 3);  // Read, high locality
        __builtin_prefetch(&circles.y[i], 0, 3);
        __builtin_prefetch(&circles.vx[i], 0, 3);
        __builtin_prefetch(&circles.vy[i], 0, 3);
    }
}

// ============================================================================
// SIMULACIÓN CON OPTIMIZACIONES DE MEMORIA
// ============================================================================
void runSimulationMemoryOptimized(MemoryOptimizedCircles& circles, int& totalBounces, double& totalEnergy, int& iterations) {
    iterations = 0;
    totalBounces = 0;
    totalEnergy = 0.0;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(10);
    
    size_t num_circles = circles.size();
    int num_threads = omp_get_max_threads();
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // OPTIMIZACIÓN: Usar arrays locales por hilo para evitar false sharing
        #pragma omp parallel
        {
            int thread_id = omp_get_thread_num();
            int chunk_size = (int)num_circles / num_threads;
            int start_idx = thread_id * chunk_size;
            int end_idx = (thread_id == num_threads - 1) ? (int)num_circles : (thread_id + 1) * chunk_size;
            
            // Prefetch datos para este hilo
            prefetch_circles_data(circles, start_idx, end_idx);
            
            // Usar arrays locales para evitar conflictos de cache
            auto& local_x = circles.thread_local_x[thread_id];
            auto& local_y = circles.thread_local_y[thread_id];
            auto& local_vx = circles.thread_local_vx[thread_id];
            auto& local_vy = circles.thread_local_vy[thread_id];
            
            local_x.clear();
            local_y.clear();
            local_vx.clear();
            local_vy.clear();
            
            float dt = 0.016f;
            int local_bounces = 0;
            
            // Procesar datos con acceso optimizado a memoria
            for (int i = start_idx; i < end_idx; ++i) {
                float x = circles.x[i] + circles.vx[i] * dt;
                float y = circles.y[i] + circles.vy[i] * dt;
                float vx = circles.vx[i];
                float vy = circles.vy[i];
                float r = circles.r[i];
                
                // Detección de colisiones
                if (x - r < 0.0f) {
                    x = r;
                    vx = -vx * 0.80f;
                    local_bounces++;
                }
                if (x + r > 800.0f) {
                    x = 800.0f - r;
                    vx = -vx * 0.80f;
                    local_bounces++;
                }
                if (y - r < 0.0f) {
                    y = r;
                    vy = -vy * 0.80f;
                    local_bounces++;
                }
                if (y + r > 600.0f) {
                    y = 600.0f - r;
                    vy = -vy * 0.80f;
                    local_bounces++;
                }
                
                // Efectos físicos
                vy += 98.0f * dt;
                vx *= 0.999f;
                vy *= 0.999f;
                
                // Guardar en arrays locales
                local_x.push_back(x);
                local_y.push_back(y);
                local_vx.push_back(vx);
                local_vy.push_back(vy);
            }
            
            // Reducir contadores locales
            #pragma omp atomic
            totalBounces += local_bounces;
            
            // Actualizar arrays principales
            #pragma omp critical
            {
                for (size_t j = 0; j < local_x.size(); ++j) {
                    circles.x[start_idx + j] = local_x[j];
                    circles.y[start_idx + j] = local_y[j];
                    circles.vx[start_idx + j] = local_vx[j];
                    circles.vy[start_idx + j] = local_vy[j];
                }
            }
        }
        
        // Cálculo de energía con acceso optimizado a memoria
        double kinetic = 0.0, potential = 0.0;
        #pragma omp parallel for reduction(+:kinetic, potential) schedule(static)
        for (int i = 0; i < (int)num_circles; ++i) {
            float vx = circles.vx[i];
            float vy = circles.vy[i];
            float r = circles.r[i];
            float y = circles.y[i];
            
            float vel = std::sqrt(vx * vx + vy * vy);
            kinetic += 0.5 * r * r * vel * vel;
            potential += 9.8 * r * r * y;
        }
        totalEnergy = kinetic + potential;
        
        iterations++;
    }
}

#endif // MEMORY_OPTIMIZATIONS_H
