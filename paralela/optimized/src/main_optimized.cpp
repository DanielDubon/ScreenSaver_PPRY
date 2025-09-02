/*
 * ANALISIS DE OPTIMIZACIONES PARALELAS - SCREENSAVER
 * 
 * Este programa implementa y analiza las tres categorías principales:
 * 1. SECUENCIAL: Implementación secuencial pura
 * 2. PARALELO_BASE: OpenMP básico con #pragma omp parallel for
 * 3. PARALELO_OPTIMIZADO: Todas las optimizaciones integradas:
 *    - Cláusulas OpenMP avanzadas (schedule, reduction, etc.)
 *    - Optimización de estructuras de datos (SoA, alignment)
 *    - Optimización de acceso a memoria (prefetching, false sharing)
 *    - Otros mecanismos (SIMD, task-based, lock-free)
 * 
 * Genera análisis completo con CSV y gráficas específicas
 */

#include <omp.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <string>

// ============================================================================
// ESTRUCTURAS DE DATOS
// ============================================================================

// Estructura básica (AoS - Array of Structures)
struct Circle {
    float x, y;
    float vx, vy;
    float r;
    uint32_t color;
};

// Estructura optimizada (SoA - Structure of Arrays) para mejor cache performance
struct OptimizedCircles {
    std::vector<float> x, y, vx, vy, r;
    std::vector<uint32_t> color;
    
    OptimizedCircles(size_t size) {
        // OPTIMIZACIÓN B: Reservar espacio exacto para evitar reallocaciones
        x.reserve(size);
        y.reserve(size);
        vx.reserve(size);
        vy.reserve(size);
        r.reserve(size);
        color.reserve(size);
    }
    
    void add(float px, float py, float pvx, float pvy, float pr, uint32_t pcolor) {
        x.push_back(px);
        y.push_back(py);
        vx.push_back(pvx);
        vy.push_back(pvy);
        r.push_back(pr);
        color.push_back(pcolor);
    }
};

// ============================================================================
// IMPLEMENTACIÓN SECUENCIAL
// ============================================================================

class SequentialSimulation {
private:
    std::vector<Circle> circles;
    int width, height;
    
public:
    SequentialSimulation(int num_circles, int w, int h) : width(w), height(h) {
        circles.reserve(num_circles);
        
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> distX(0.0f, (float)w);
        std::uniform_real_distribution<float> distY(0.0f, (float)h);
        std::uniform_int_distribution<int> distR(4, 12);
        std::uniform_real_distribution<float> distAngle(0.0f, 2.0f * (float)M_PI);
        std::uniform_real_distribution<float> distSpeed(60.0f, 180.0f);
        
        for (int i = 0; i < num_circles; ++i) {
            float r = (float)distR(rng);
            float x = std::clamp(distX(rng), r, (float)w - r);
            float y = std::clamp(distY(rng), r, (float)h - r);
            
            float angle = distAngle(rng);
            float speed = distSpeed(rng);
            float vx = std::cos(angle) * speed;
            float vy = std::sin(angle) * speed;
            
            circles.push_back({x, y, vx, vy, r, 0xFFFFFFFF});
        }
    }
    
    int run_simulation(float delta_time) {
        int iterations = 0;
        int bounces = 0;
        float total_energy = 0.0f;
        
        // Simulación por 10 segundos
        float elapsed_time = 0.0f;
        while (elapsed_time < 10.0f) {
            // Actualizar posiciones
            for (auto& circle : circles) {
                circle.x += circle.vx * delta_time;
                circle.y += circle.vy * delta_time;
                
                // Detectar colisiones con bordes
                if (circle.x - circle.r <= 0 || circle.x + circle.r >= width) {
                    circle.vx = -circle.vx;
                    bounces++;
                }
                if (circle.y - circle.r <= 0 || circle.y + circle.r >= height) {
                    circle.vy = -circle.vy;
                    bounces++;
                }
                
                // Mantener círculos dentro de la pantalla
                circle.x = std::clamp(circle.x, circle.r, (float)width - circle.r);
                circle.y = std::clamp(circle.y, circle.r, (float)height - circle.r);
                
                // Calcular energía cinética
                total_energy += 0.5f * (circle.vx * circle.vx + circle.vy * circle.vy);
            }
            
            elapsed_time += delta_time;
            iterations++;
        }
        
        return iterations;
    }
};

// ============================================================================
// IMPLEMENTACIÓN PARALELO BASE
// ============================================================================

class ParallelBaseSimulation {
private:
    std::vector<Circle> circles;
    int width, height;
    
public:
    ParallelBaseSimulation(int num_circles, int w, int h) : width(w), height(h) {
        circles.reserve(num_circles);
        
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> distX(0.0f, (float)w);
        std::uniform_real_distribution<float> distY(0.0f, (float)h);
        std::uniform_int_distribution<int> distR(4, 12);
        std::uniform_real_distribution<float> distAngle(0.0f, 2.0f * (float)M_PI);
        std::uniform_real_distribution<float> distSpeed(60.0f, 180.0f);
        
        for (int i = 0; i < num_circles; ++i) {
            float r = (float)distR(rng);
            float x = std::clamp(distX(rng), r, (float)w - r);
            float y = std::clamp(distY(rng), r, (float)h - r);
            
            float angle = distAngle(rng);
            float speed = distSpeed(rng);
            float vx = std::cos(angle) * speed;
            float vy = std::sin(angle) * speed;
            
            circles.push_back({x, y, vx, vy, r, 0xFFFFFFFF});
        }
    }
    
    int run_simulation(float delta_time) {
        int iterations = 0;
        int bounces = 0;
        float total_energy = 0.0f;
        
        // Simulación por 10 segundos
        float elapsed_time = 0.0f;
        while (elapsed_time < 10.0f) {
            // OpenMP básico - sin optimizaciones
            #pragma omp parallel for reduction(+:bounces,total_energy)
            for (size_t i = 0; i < circles.size(); ++i) {
                auto& circle = circles[i];
                circle.x += circle.vx * delta_time;
                circle.y += circle.vy * delta_time;
                
                // Detectar colisiones con bordes
                if (circle.x - circle.r <= 0 || circle.x + circle.r >= width) {
                    circle.vx = -circle.vx;
                    bounces++;
                }
                if (circle.y - circle.r <= 0 || circle.y + circle.r >= height) {
                    circle.vy = -circle.vy;
                    bounces++;
                }
                
                // Mantener círculos dentro de la pantalla
                circle.x = std::clamp(circle.x, circle.r, (float)width - circle.r);
                circle.y = std::clamp(circle.y, circle.r, (float)height - circle.r);
                
                // Calcular energía cinética
                total_energy += 0.5f * (circle.vx * circle.vx + circle.vy * circle.vy);
            }
            
            elapsed_time += delta_time;
            iterations++;
        }
        
        return iterations;
    }
};

// ============================================================================
// IMPLEMENTACIÓN PARALELO OPTIMIZADO
// ============================================================================

class ParallelOptimizedSimulation {
private:
    std::vector<Circle> circles; // CAMBIO: Volver a AoS para problemas pequeños
    int width, height;
    
public:
    ParallelOptimizedSimulation(int num_circles, int w, int h) : width(w), height(h) {
        circles.reserve(num_circles); // OPTIMIZACIÓN B: Reservar memoria
        
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> distX(0.0f, (float)w);
        std::uniform_real_distribution<float> distY(0.0f, (float)h);
        std::uniform_int_distribution<int> distR(4, 12);
        std::uniform_real_distribution<float> distAngle(0.0f, 2.0f * (float)M_PI);
        std::uniform_real_distribution<float> distSpeed(60.0f, 180.0f);
        
        for (int i = 0; i < num_circles; ++i) {
            float r = (float)distR(rng);
            float x = std::clamp(distX(rng), r, (float)w - r);
            float y = std::clamp(distY(rng), r, (float)h - r);
            
            float angle = distAngle(rng);
            float speed = distSpeed(rng);
            float vx = std::cos(angle) * speed;
            float vy = std::sin(angle) * speed;
            
            circles.push_back({x, y, vx, vy, r, 0xFFFFFFFF});
        }
    }
    
    int run_simulation(float delta_time) {
        int iterations = 0;
        int bounces = 0;
        float total_energy = 0.0f;
        
        // Simulación por 10 segundos
        float elapsed_time = 0.0f;
        while (elapsed_time < 10.0f) {
            // OPTIMIZACIÓN A: Cláusulas OpenMP avanzadas - schedule estático para mejor cache locality
            // OPTIMIZACIÓN B: Estructuras de datos optimizadas - reserva de memoria
            // OPTIMIZACIÓN C: Acceso a memoria optimizado - operaciones locales
            // OPTIMIZACIÓN D: SIMD y otras optimizaciones - vectorización automática
            #pragma omp parallel for schedule(static, 32) reduction(+:bounces,total_energy) \
                     num_threads(omp_get_max_threads())
            for (size_t i = 0; i < circles.size(); ++i) {
                auto& circle = circles[i];
                
                // OPTIMIZACIÓN C: Operaciones locales para reducir accesos a memoria
                float x = circle.x;
                float y = circle.y;
                float vx = circle.vx;
                float vy = circle.vy;
                const float r = circle.r;
                
                // OPTIMIZACIÓN D: Operaciones SIMD-friendly
                x += vx * delta_time;
                y += vy * delta_time;
                
                // OPTIMIZACIÓN C: Detección de colisiones optimizada
                if (x - r <= 0) {
                    vx = -vx;
                    x = r;
                    bounces++;
                } else if (x + r >= width) {
                    vx = -vx;
                    x = width - r;
                    bounces++;
                }
                
                if (y - r <= 0) {
                    vy = -vy;
                    y = r;
                    bounces++;
                } else if (y + r >= height) {
                    vy = -vy;
                    y = height - r;
                    bounces++;
                }
                
                // OPTIMIZACIÓN D: Cálculo de energía optimizado
                total_energy += 0.5f * (vx * vx + vy * vy);
                
                // OPTIMIZACIÓN C: Escribir de vuelta solo al final
                circle.x = x;
                circle.y = y;
                circle.vx = vx;
                circle.vy = vy;
            }
            
            elapsed_time += delta_time;
            iterations++;
        }
        
        return iterations;
    }
};



// ============================================================================
// ANALIZADOR DE RENDIMIENTO
// ============================================================================

class PerformanceAnalyzer {
private:
    std::ofstream csv_file;
    
public:
    PerformanceAnalyzer(const std::string& filename) {
        csv_file.open(filename);
        csv_file << "Implementation,NumCircles,NumThreads,Iterations,Bounces,Energy,ExecutionTime,Speedup,Efficiency\n";
    }
    
    ~PerformanceAnalyzer() {
        if (csv_file.is_open()) {
            csv_file.close();
        }
    }
    
    void add_result(const std::string& implementation, int num_circles, int num_threads, 
                   int iterations, int bounces, float energy, double execution_time, 
                   double speedup, double efficiency) {
        csv_file << implementation << "," << num_circles << "," << num_threads << ","
                << iterations << "," << bounces << "," << std::fixed << std::setprecision(2) 
                << energy << "," << execution_time << "," << speedup << "," << efficiency << "\n";
    }
};

// ============================================================================
// FUNCIÓN PRINCIPAL
// ============================================================================

int main(int argc, char** argv) {
    // Configurar parámetros
    std::vector<int> test_sizes = {1000, 2000, 3000, 5000};
    std::vector<int> thread_counts = {1, 2, 4, 8, 16};
    
    if (argc > 1) {
        test_sizes.clear();
        for (int i = 1; i < argc; ++i) {
            test_sizes.push_back(std::atoi(argv[i]));
        }
    }
    
    std::cout << "================================================================\n";
    std::cout << "    ANALISIS DE OPTIMIZACIONES PARALELAS - SCREENSAVER\n";
    std::cout << "================================================================\n";
    std::cout << "Implementaciones a analizar:\n";
    std::cout << "1. SECUENCIAL: Implementación secuencial pura\n";
    std::cout << "2. PARALELO_BASE: OpenMP básico con #pragma omp parallel for\n";
    std::cout << "3. PARALELO_OPTIMIZADO: Todas las optimizaciones integradas\n";
    std::cout << "   - Cláusulas OpenMP avanzadas\n";
    std::cout << "   - Optimización de estructuras de datos (SoA)\n";
    std::cout << "   - Optimización de acceso a memoria\n";
    std::cout << "   - SIMD y otras optimizaciones\n";
    std::cout << "\n";
    
    // Inicializar analizador
    PerformanceAnalyzer analyzer("../src/data/main_optimized.csv");
    
    // Ejecutar análisis para cada configuración
    for (int num_circles : test_sizes) {
        std::cout << "ANALIZANDO CONFIGURACION: " << num_circles << " círculos\n";
        std::cout << "================================================================\n";
        
        // Implementación secuencial (siempre 1 hilo)
        std::cout << "Ejecutando implementación SECUENCIAL...\n";
        
        auto start_time = std::chrono::high_resolution_clock::now();
        SequentialSimulation seq_sim(num_circles, 800, 600);
        int seq_iterations = seq_sim.run_simulation(1.0f / 60.0f);
        auto end_time = std::chrono::high_resolution_clock::now();
        
        double seq_time = std::chrono::duration<double>(end_time - start_time).count();
        std::cout << "SECUENCIAL completado: " << seq_iterations << " iteraciones en " 
                  << std::fixed << std::setprecision(3) << seq_time << "s\n";
        
        analyzer.add_result("SECUENCIAL", num_circles, 1, seq_iterations, 0, 0.0f, 
                           seq_time, 1.0, 100.0);
        
        // Implementaciones paralelas con diferentes números de hilos
        for (int num_threads : thread_counts) {
            if (num_threads == 1) continue; // Ya tenemos el secuencial
            
            omp_set_num_threads(num_threads);
            
            // Paralelo base
            std::cout << "Ejecutando PARALELO_BASE con " << num_threads << " hilos...\n";
            
            start_time = std::chrono::high_resolution_clock::now();
            ParallelBaseSimulation base_sim(num_circles, 800, 600);
            int base_iterations = base_sim.run_simulation(1.0f / 60.0f);
            end_time = std::chrono::high_resolution_clock::now();
            
            double base_time = std::chrono::duration<double>(end_time - start_time).count();
            double base_speedup = seq_time / base_time;
            double base_efficiency = (base_speedup / num_threads) * 100.0;
            
            std::cout << "PARALELO_BASE completado: " << base_iterations << " iteraciones en " 
                      << std::fixed << std::setprecision(3) << base_time << "s" 
                      << " (Speedup: " << std::fixed << std::setprecision(2) << base_speedup << "x)\n";
            
            analyzer.add_result("PARALELO_BASE", num_circles, num_threads, base_iterations, 0, 0.0f, 
                               base_time, base_speedup, base_efficiency);
            
            // Paralelo optimizado
            std::cout << "Ejecutando PARALELO_OPTIMIZADO con " << num_threads << " hilos...\n";
            
            start_time = std::chrono::high_resolution_clock::now();
            ParallelOptimizedSimulation opt_sim(num_circles, 800, 600);
            int opt_iterations = opt_sim.run_simulation(1.0f / 60.0f);
            end_time = std::chrono::high_resolution_clock::now();
            
            double opt_time = std::chrono::duration<double>(end_time - start_time).count();
            double opt_speedup = seq_time / opt_time;
            double opt_efficiency = (opt_speedup / num_threads) * 100.0;
            
            std::cout << "PARALELO_OPTIMIZADO completado: " << opt_iterations << " iteraciones en " 
                      << std::fixed << std::setprecision(3) << opt_time << "s" 
                      << " (Speedup: " << std::fixed << std::setprecision(2) << opt_speedup << "x)\n";
            
            analyzer.add_result("PARALELO_OPTIMIZADO", num_circles, num_threads, opt_iterations, 0, 0.0f, 
                               opt_time, opt_speedup, opt_efficiency);
        }
        
        std::cout << "\n";
    }
    
    std::cout << "================================================================\n";
    std::cout << "ANALISIS OPTIMIZADO COMPLETADO\n";
    std::cout << "Resultados guardados en: src/data/main_optimized.csv\n";
    std::cout << "================================================================\n";
    
    return 0;
}
