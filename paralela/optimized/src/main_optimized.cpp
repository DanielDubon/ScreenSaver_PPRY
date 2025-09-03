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
 
 // ESTRUCTURAS DE DATOS
 
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
 
 // IMPLEMENTACIÓN SECUENCIAL
 
 class SequentialSimulation {
 private:
     std::vector<Circle> circles;
     int width, height;
     
 public:
     SequentialSimulation(int num_circles, int w, int h, unsigned int seed = 0) : width(w), height(h) {
         circles.reserve(num_circles);
         
         // Usar seed diferente para cada medición si se proporciona
         std::mt19937 rng(seed == 0 ? std::random_device{}() : seed);
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
     
     int run_simulation(float delta_time, float simulation_duration = 30.0f) {
         int iterations = 0;
         int bounces = 0;
         float total_energy = 0.0f;
         
         // Simulación por duración especificada
         float elapsed_time = 0.0f;
         while (elapsed_time < simulation_duration) {
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
 
 // IMPLEMENTACIÓN PARALELO BASE
 
 class ParallelBaseSimulation {
 private:
     std::vector<Circle> circles;
     int width, height;
     
 public:
     ParallelBaseSimulation(int num_circles, int w, int h, unsigned int seed = 0) : width(w), height(h) {
         circles.reserve(num_circles);
         
         std::mt19937 rng(seed == 0 ? std::random_device{}() : seed);
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
     
     int run_simulation(float delta_time, float simulation_duration = 30.0f) {
         int iterations = 0;
         int bounces = 0;
         float total_energy = 0.0f;
         
         // Simulación por duración especificada
         float elapsed_time = 0.0f;
         while (elapsed_time < simulation_duration) {
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
 
 // IMPLEMENTACIÓN PARALELO OPTIMIZADO
 
 class ParallelOptimizedSimulation {
 private:
     std::vector<Circle> circles;
     int width, height;
     
 public:
     ParallelOptimizedSimulation(int num_circles, int w, int h, unsigned int seed = 0) : width(w), height(h) {
         circles.reserve(num_circles); // OPTIMIZACIÓN B: Reservar memoria
         
         std::mt19937 rng(seed == 0 ? std::random_device{}() : seed);
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
     
     int run_simulation(float delta_time, float simulation_duration = 30.0f) {
         int iterations = 0;
         int bounces = 0;
         float total_energy = 0.0f;
         
         // Simulación por duración especificada
         float elapsed_time = 0.0f;
         while (elapsed_time < simulation_duration) {
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
 
 // ANALIZADOR DE RENDIMIENTO
 
 class PerformanceAnalyzer {
 private:
     std::ofstream csv_file;
     
 public:
     PerformanceAnalyzer(const std::string& filename) {
         csv_file.open(filename);
         csv_file << "Implementation,NumCircles,NumThreads,Repetition,Iterations,Bounces,Energy,ExecutionTime,Speedup,Efficiency\n";
     }
     
     ~PerformanceAnalyzer() {
         if (csv_file.is_open()) {
             csv_file.close();
         }
     }
     
     void add_result(const std::string& implementation, int num_circles, int num_threads, 
                    int repetition, int iterations, int bounces, float energy, 
                    double execution_time, double speedup, double efficiency) {
         csv_file << implementation << "," << num_circles << "," << num_threads << ","
                 << repetition << "," << iterations << "," << bounces << "," 
                 << std::fixed << std::setprecision(2) << energy << "," 
                 << execution_time << "," << speedup << "," << efficiency << "\n";
     }
 };
 
 // FUNCIÓN PRINCIPAL
 
 int main(int argc, char** argv) {
     // Configurar parámetros más grandes para obtener tiempos medibles
     std::vector<int> test_sizes = {5000, 10000, 20000, 50000};
     std::vector<int> thread_counts = {1, 2, 4, 8, 16};
     const int NUM_REPETITIONS = 10; // 10 mediciones por configuración
     const float SIMULATION_DURATION = 30.0f; // 30 segundos por simulación
     
     if (argc > 1) {
         test_sizes.clear();
         for (int i = 1; i < argc; ++i) {
             test_sizes.push_back(std::atoi(argv[i]));
         }
     }
     
     std::cout << "================================================================\n";
     std::cout << "    ANALISIS DE OPTIMIZACIONES PARALELAS - SCREENSAVER\n";
     std::cout << "================================================================\n";
     std::cout << "Configuración:\n";
     std::cout << "- " << NUM_REPETITIONS << " mediciones independientes por configuración\n";
     std::cout << "- Duración de simulación: " << SIMULATION_DURATION << " segundos\n";
     std::cout << "- Tamaños de problema: ";
     for (size_t i = 0; i < test_sizes.size(); ++i) {
         std::cout << test_sizes[i];
         if (i < test_sizes.size() - 1) std::cout << ", ";
     }
     std::cout << " círculos\n";
     
     int total_tests = test_sizes.size() * (1 + (thread_counts.size() - 1) * 2) * NUM_REPETITIONS;
     std::cout << "- Total de pruebas a ejecutar: " << total_tests << "\n";
     std::cout << "\nImplementaciones a analizar:\n";
     std::cout << "1. SECUENCIAL: Implementación secuencial pura\n";
     std::cout << "2. PARALELO_BASE: OpenMP básico\n";
     std::cout << "3. PARALELO_OPTIMIZADO: Todas las optimizaciones integradas\n\n";
     
     // Inicializar analizador
     PerformanceAnalyzer analyzer("../src/data/main_optimized.csv");
     
     int completed_tests = 0;
     
     // Ejecutar análisis para cada configuración
     for (int num_circles : test_sizes) {
         std::cout << "ANALIZANDO CONFIGURACION: " << num_circles << " círculos\n";
         std::cout << "================================================================\n";
         
         // Implementación secuencial (siempre 1 hilo) - 10 repeticiones
         std::cout << "Ejecutando implementación SECUENCIAL (" << NUM_REPETITIONS << " mediciones)...\n";
         
         std::vector<double> seq_times(NUM_REPETITIONS);
         int seq_iterations_avg = 0;
         
         for (int rep = 0; rep < NUM_REPETITIONS; ++rep) {
             unsigned int seed = std::chrono::steady_clock::now().time_since_epoch().count() + rep;
             
             auto start_time = std::chrono::high_resolution_clock::now();
             SequentialSimulation seq_sim(num_circles, 800, 600, seed);
             int seq_iterations = seq_sim.run_simulation(1.0f / 60.0f, SIMULATION_DURATION);
             auto end_time = std::chrono::high_resolution_clock::now();
             
             seq_times[rep] = std::chrono::duration<double>(end_time - start_time).count();
             seq_iterations_avg += seq_iterations;
             
             analyzer.add_result("SECUENCIAL", num_circles, 1, rep + 1, seq_iterations, 
                                0, 0.0f, seq_times[rep], 1.0, 100.0);
             
             completed_tests++;
             if (rep < 3) {
                 std::cout << "  Repetición " << (rep + 1) << ": " 
                           << std::fixed << std::setprecision(3) << seq_times[rep] << "s\n";
             }
         }
         
         seq_iterations_avg /= NUM_REPETITIONS;
         
         // Calcular estadísticas secuenciales
         double seq_time_avg = 0.0;
         for (double time : seq_times) seq_time_avg += time;
         seq_time_avg /= NUM_REPETITIONS;
         
         double seq_time_std = 0.0;
         for (double time : seq_times) {
             seq_time_std += (time - seq_time_avg) * (time - seq_time_avg);
         }
         seq_time_std = std::sqrt(seq_time_std / (NUM_REPETITIONS - 1));
         
         std::cout << "SECUENCIAL completado - Tiempo promedio: " 
                   << std::fixed << std::setprecision(3) << seq_time_avg 
                   << "s ± " << seq_time_std << "s\n";
         
         // Implementaciones paralelas con diferentes números de hilos
         for (int num_threads : thread_counts) {
             if (num_threads == 1) continue;
             
             omp_set_num_threads(num_threads);
             
             // Paralelo base - 10 repeticiones
             std::cout << "Ejecutando PARALELO_BASE con " << num_threads 
                       << " hilos (" << NUM_REPETITIONS << " mediciones)...\n";
             
             std::vector<double> base_times(NUM_REPETITIONS);
             for (int rep = 0; rep < NUM_REPETITIONS; ++rep) {
                 unsigned int seed = std::chrono::steady_clock::now().time_since_epoch().count() + rep + 1000;
                 
                 auto start_time = std::chrono::high_resolution_clock::now();
                 ParallelBaseSimulation base_sim(num_circles, 800, 600, seed);
                 int base_iterations = base_sim.run_simulation(1.0f / 60.0f, SIMULATION_DURATION);
                 auto end_time = std::chrono::high_resolution_clock::now();
                 
                 base_times[rep] = std::chrono::duration<double>(end_time - start_time).count();
                 double speedup = seq_time_avg / base_times[rep];
                 double efficiency = (speedup / num_threads) * 100.0;
                 
                 analyzer.add_result("PARALELO_BASE", num_circles, num_threads, rep + 1, 
                                    base_iterations, 0, 0.0f, base_times[rep], speedup, efficiency);
                 
                 completed_tests++;
                 if (rep < 3) {
                     std::cout << "  Repetición " << (rep + 1) << ": " 
                               << std::fixed << std::setprecision(3) << base_times[rep] 
                               << "s (Speedup: " << std::setprecision(2) << speedup << "x)\n";
                 }
             }
             
             // Calcular promedio para reporte
             double base_time_avg = 0.0;
             for (double time : base_times) base_time_avg += time;
             base_time_avg /= NUM_REPETITIONS;
             double base_speedup_avg = seq_time_avg / base_time_avg;
             
             std::cout << "PARALELO_BASE completado - Speedup promedio: " 
                       << std::fixed << std::setprecision(2) << base_speedup_avg << "x\n";
             
             // Paralelo optimizado - 10 repeticiones
             std::cout << "Ejecutando PARALELO_OPTIMIZADO con " << num_threads 
                       << " hilos (" << NUM_REPETITIONS << " mediciones)...\n";
             
             std::vector<double> opt_times(NUM_REPETITIONS);
             for (int rep = 0; rep < NUM_REPETITIONS; ++rep) {
                 unsigned int seed = std::chrono::steady_clock::now().time_since_epoch().count() + rep + 2000;
                 
                 auto start_time = std::chrono::high_resolution_clock::now();
                 ParallelOptimizedSimulation opt_sim(num_circles, 800, 600, seed);
                 int opt_iterations = opt_sim.run_simulation(1.0f / 60.0f, SIMULATION_DURATION);
                 auto end_time = std::chrono::high_resolution_clock::now();
                 
                 opt_times[rep] = std::chrono::duration<double>(end_time - start_time).count();
                 double speedup = seq_time_avg / opt_times[rep];
                 double efficiency = (speedup / num_threads) * 100.0;
                 
                 analyzer.add_result("PARALELO_OPTIMIZADO", num_circles, num_threads, rep + 1, 
                                    opt_iterations, 0, 0.0f, opt_times[rep], speedup, efficiency);
                 
                 completed_tests++;
                 if (rep < 3) {
                     std::cout << "  Repetición " << (rep + 1) << ": " 
                               << std::fixed << std::setprecision(3) << opt_times[rep] 
                               << "s (Speedup: " << std::setprecision(2) << speedup << "x)\n";
                 }
             }
             
             // Calcular promedio para reporte
             double opt_time_avg = 0.0;
             for (double time : opt_times) opt_time_avg += time;
             opt_time_avg /= NUM_REPETITIONS;
             double opt_speedup_avg = seq_time_avg / opt_time_avg;
             
             std::cout << "PARALELO_OPTIMIZADO completado - Speedup promedio: " 
                       << std::fixed << std::setprecision(2) << opt_speedup_avg << "x\n";
         }
         
         std::cout << "Progreso: " << completed_tests << "/" << total_tests << " pruebas completadas\n\n";
     }

     std::cout << "ANALISIS OPTIMIZADO COMPLETADO\n";
     std::cout << "Total de mediciones realizadas: " << completed_tests << "\n";
     
     return 0;
 }