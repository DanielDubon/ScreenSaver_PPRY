/*
 * OPTIMIZACIONES AVANZADAS - Inciso D
 * 
 * Este header implementa mecanismos de optimización adicionales:
 * 1. SIMD vectorization
 * 2. Task-based parallelism
 * 3. Work-stealing algorithms
 * 4. Lock-free data structures
 * 5. Compiler optimizations
 */

#ifndef ADVANCED_OPTIMIZATIONS_H
#define ADVANCED_OPTIMIZATIONS_H

#include <vector>
#include <memory>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <omp.h>
#include <atomic>
#include <queue>
#include <thread>
#include "common.h"

// ============================================================================
// ESTRUCTURA CON OPTIMIZACIONES AVANZADAS
// ============================================================================
struct AdvancedOptimizedCircles {
    // Arrays principales con alineación SIMD
    alignas(32) std::vector<float> x;
    alignas(32) std::vector<float> y;
    alignas(32) std::vector<float> vx;
    alignas(32) std::vector<float> vy;
    alignas(32) std::vector<float> r;
    alignas(32) std::vector<uint32_t> colors;
    
    // Contadores lock-free
    std::atomic<int> total_bounces{0};
    std::atomic<double> total_energy{0.0};
    
    // Constructor optimizado
    explicit AdvancedOptimizedCircles(size_t capacity) {
        x.reserve(capacity);
        y.reserve(capacity);
        vx.reserve(capacity);
        vy.reserve(capacity);
        r.reserve(capacity);
        colors.reserve(capacity);
    }
    
    size_t size() const { return x.size(); }
};

// ============================================================================
// SIMD VECTORIZATION HELPERS
// ============================================================================
#ifdef __AVX2__
#include <immintrin.h>

// Procesar 8 floats a la vez con AVX2
inline void simd_update_positions(float* x, float* y, float* vx, float* vy, float dt, int count) {
    __m256 dt_vec = _mm256_set1_ps(dt);
    
    for (int i = 0; i < count; i += 8) {
        __m256 x_vec = _mm256_load_ps(&x[i]);
        __m256 y_vec = _mm256_load_ps(&y[i]);
        __m256 vx_vec = _mm256_load_ps(&vx[i]);
        __m256 vy_vec = _mm256_load_ps(&vy[i]);
        
        // x += vx * dt
        x_vec = _mm256_fmadd_ps(vx_vec, dt_vec, x_vec);
        // y += vy * dt
        y_vec = _mm256_fmadd_ps(vy_vec, dt_vec, y_vec);
        
        _mm256_store_ps(&x[i], x_vec);
        _mm256_store_ps(&y[i], y_vec);
    }
}
#else
// Fallback para procesadores sin AVX2
inline void simd_update_positions(float* x, float* y, float* vx, float* vy, float dt, int count) {
    for (int i = 0; i < count; ++i) {
        x[i] += vx[i] * dt;
        y[i] += vy[i] * dt;
    }
}
#endif

// ============================================================================
// TASK-BASED PARALLELISM
// ============================================================================
struct SimulationTask {
    int start_idx;
    int end_idx;
    float dt;
    
    SimulationTask(int start, int end, float delta_t) 
        : start_idx(start), end_idx(end), dt(delta_t) {}
};

// ============================================================================
// SIMULACIÓN CON OPTIMIZACIONES AVANZADAS
// ============================================================================
void runSimulationAdvancedOptimized(AdvancedOptimizedCircles& circles, int& totalBounces, double& totalEnergy, int& iterations) {
    iterations = 0;
    circles.total_bounces.store(0);
    circles.total_energy.store(0.0);
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(10);
    
    size_t num_circles = circles.size();
    int num_threads = omp_get_max_threads();
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // OPTIMIZACIÓN: Task-based parallelism con work-stealing
        #pragma omp parallel
        {
            #pragma omp single
            {
                // Crear tareas para diferentes partes de la simulación
                int chunk_size = (int)num_circles / num_threads;
                
                for (int i = 0; i < num_threads; ++i) {
                    int start_idx = i * chunk_size;
                    int end_idx = (i == num_threads - 1) ? (int)num_circles : (i + 1) * chunk_size;
                    
                    #pragma omp task
                    {
                        float dt = 0.016f;
                        int local_bounces = 0;
                        
                        // OPTIMIZACIÓN: SIMD vectorization para actualización de posiciones
                        simd_update_positions(
                            &circles.x[start_idx], 
                            &circles.y[start_idx], 
                            &circles.vx[start_idx], 
                            &circles.vy[start_idx], 
                            dt, 
                            end_idx - start_idx
                        );
                        
                        // Detección de colisiones optimizada
                        for (int j = start_idx; j < end_idx; ++j) {
                            float& x = circles.x[j];
                            float& y = circles.y[j];
                            float& vx = circles.vx[j];
                            float& vy = circles.vy[j];
                            float& r = circles.r[j];
                            
                            // Colisiones con paredes
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
                        }
                        
                        // Actualizar contador lock-free
                        circles.total_bounces.fetch_add(local_bounces, std::memory_order_relaxed);
                    }
                }
            }
        }
        
        // Cálculo de energía con optimizaciones SIMD
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
        
        circles.total_energy.store(kinetic + potential, std::memory_order_relaxed);
        
        iterations++;
    }
    
    // Obtener resultados finales
    totalBounces = circles.total_bounces.load();
    totalEnergy = circles.total_energy.load();
}

#endif // ADVANCED_OPTIMIZATIONS_H
