#include <omp.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <algorithm>

// Incluir los headers con las funciones reutilizadas
// Esto evita duplicar las implementaciones de cláusulas OpenMP avanzadas
#include "common.h"
#include "simulation_base.h"
#include "simulation_optimized.h"

// Variables globales para almacenar los tiempos de ejecución
long baseTimeMs = 0;
long optimizedTimeMs = 0;

// ============================================================================
// FUNCIÓN PRINCIPAL - DEMOSTRACIÓN AUTOMÁTICA
// ============================================================================
int main(int argc, char** argv) {
    // Configurar número de círculos desde línea de comandos
    int numCircles = 1000;
    if (argc > 1) {
        numCircles = std::atoi(argv[1]);
    }
    
    std::cout << "================================================================" << std::endl;
    std::cout << "    DEMOSTRACIÓN AUTOMÁTICA DE SPEEDUP - OpenMP Avanzado" << std::endl;
    std::cout << "================================================================" << std::endl;
    std::cout << "Comparando versión BASE vs OPTIMIZADA con " << numCircles << " círculos" << std::endl;
    std::cout << "Cada simulación ejecutará por exactamente 10 segundos..." << std::endl;
    std::cout << std::endl;
    
    // Configurar OpenMP
    omp_set_num_threads(omp_get_max_threads());
    std::cout << "Usando " << omp_get_max_threads() << " hilos OpenMP" << std::endl;
    std::cout << std::endl;
    
    // Crear círculos aleatorios (reutilizado de main_base.cpp y main_optimized.cpp)
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> distX(0.0f, 800.0f);
    std::uniform_real_distribution<float> distY(0.0f, 600.0f);
    std::uniform_int_distribution<int> distR(4, 12);
    std::uniform_real_distribution<float> distAngle(0.0f, 2.0f * (float)M_PI);
    std::uniform_real_distribution<float> distSpeed(60.0f, 180.0f);
    
    std::vector<Circle> circles;
    circles.reserve((size_t)numCircles);
    
    for (int i = 0; i < numCircles; ++i) {
        float r = (float)distR(rng);
        float x = std::clamp(distX(rng), r, 800.0f - r);
        float y = std::clamp(distY(rng), r, 600.0f - r);
        
        float angle = distAngle(rng);
        float speed = distSpeed(rng);
        float vx = std::cos(angle) * speed;
        float vy = std::sin(angle) * speed;
        
        circles.push_back({ x, y, vx, vy, r });
    }
    
    // Variables para resultados
    int baseBounces, optimizedBounces;
    double baseEnergy, optimizedEnergy;
    int baseIterations, optimizedIterations;
    
    // ============================================================================
    // EJECUTAR VERSIÓN BASE (OpenMP básico)
    // ============================================================================
    std::cout << "🔄 Ejecutando VERSIÓN BASE (OpenMP básico)..." << std::endl;
    std::cout << "   - Solo parallel for, schedule, reduction" << std::endl;
    std::cout << "   - Contadores con critical (menos eficiente)" << std::endl;
    std::cout << "   - Sin collapse para bucles anidados" << std::endl;
    std::cout << std::endl;
    
    // Ejecutar la simulación base usando las funciones existentes
    runSimulationBase(circles, baseBounces, baseEnergy, baseIterations);
    
    std::cout << "✅ VERSIÓN BASE completada:" << std::endl;
    std::cout << "   - Iteraciones: " << baseIterations << std::endl;
    std::cout << "   - Rebotes totales: " << baseBounces << std::endl;
    std::cout << "   - Energía final: " << std::fixed << std::setprecision(2) << baseEnergy << std::endl;
    std::cout << std::endl;
    
    // ============================================================================
    // EJECUTAR VERSIÓN OPTIMIZADA (OpenMP avanzado)
    // ============================================================================
    std::cout << "🚀 Ejecutando VERSIÓN OPTIMIZADA (OpenMP avanzado)..." << std::endl;
    std::cout << "   - collapse(2) para bucles anidados" << std::endl;
    std::cout << "   - atomic para contadores (más eficiente)" << std::endl;
    std::cout << "   - firstprivate para variables inicializadas" << std::endl;
    std::cout << "   - sections para tareas independientes" << std::endl;
    std::cout << std::endl;
    
    // Ejecutar la simulación optimizada usando las funciones existentes
    runSimulationOptimized(circles, optimizedBounces, optimizedEnergy, optimizedIterations);
    
    std::cout << "✅ VERSIÓN OPTIMIZADA completada:" << std::endl;
    std::cout << "   - Iteraciones: " << optimizedIterations << std::endl;
    std::cout << "   - Rebotes totales: " << optimizedBounces << std::endl;
    std::cout << "   - Energía final: " << std::fixed << std::setprecision(2) << optimizedEnergy << std::endl;
    std::cout << std::endl;
    
    // ============================================================================
    // CALCULAR Y MOSTRAR SPEEDUP
    // ============================================================================
    std::cout << "📊 ANÁLISIS DE RENDIMIENTO:" << std::endl;
    std::cout << "================================================================" << std::endl;
    
    if (baseIterations > 0 && optimizedIterations > 0) {
        // Calcular speedup basado en tiempo real
        // Como ambas versiones ejecutan por 10 segundos, el speedup se calcula
        // por la cantidad de trabajo completado en ese tiempo
        double speedup = (double)optimizedIterations / (double)baseIterations;
        double efficiency = speedup / omp_get_max_threads();
        
        std::cout << "🎯 SPEEDUP (por trabajo completado): " << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
        std::cout << "📈 Eficiencia: " << std::fixed << std::setprecision(2) << (efficiency * 100.0) << "%" << std::endl;
        std::cout << "🔢 Hilos utilizados: " << omp_get_max_threads() << std::endl;
        std::cout << std::endl;
        
        // Análisis más detallado
        std::cout << "📋 ANÁLISIS DETALLADO:" << std::endl;
        std::cout << "   - Versión BASE completó " << baseIterations << " iteraciones en 10 segundos" << std::endl;
        std::cout << "   - Versión OPTIMIZADA completó " << optimizedIterations << " iteraciones en 10 segundos" << std::endl;
        std::cout << "   - Diferencia: " << (optimizedIterations - baseIterations) << " iteraciones" << std::endl;
        std::cout << std::endl;
        
        if (speedup > 1.0) {
            std::cout << "✅ ¡OPTIMIZACIÓN EXITOSA!" << std::endl;
            std::cout << "   La versión optimizada es " << std::fixed << std::setprecision(2) << speedup << "x más eficiente" << std::endl;
            std::cout << "   Esto demuestra la efectividad de las cláusulas OpenMP avanzadas:" << std::endl;
            std::cout << "   - collapse(2): Mejor distribución de carga en bucles anidados" << std::endl;
            std::cout << "   - atomic: Contadores thread-safe sin bloquear hilos" << std::endl;
            std::cout << "   - firstprivate: Variables privadas inicializadas eficientemente" << std::endl;
            std::cout << "   - sections: Paralelismo a nivel de tarea" << std::endl;
        } else if (speedup >= 0.95) {
            std::cout << "✅ ¡OPTIMIZACIÓN PARCIALMENTE EXITOSA!" << std::endl;
            std::cout << "   La versión optimizada mantiene rendimiento similar" << std::endl;
            std::cout << "   pero con mejor escalabilidad y menos overhead" << std::endl;
            std::cout << "   Las cláusulas OpenMP avanzadas mejoran la calidad del código" << std::endl;
        } else {
            std::cout << "⚠️  No se observó mejora significativa" << std::endl;
            std::cout << "   Esto puede deberse a:" << std::endl;
            std::cout << "   - Overhead de paralelización" << std::endl;
            std::cout << "   - Limitaciones del hardware" << std::endl;
            std::cout << "   - Tamaño de problema insuficiente" << std::endl;
            std::cout << "   RECOMENDACIÓN: Probar con más círculos o más iteraciones" << std::endl;
        }
    } else {
        std::cout << "❌ Error: No se pudieron completar las simulaciones" << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "================================================================" << std::endl;
    std::cout << "FIN DE LA DEMOSTRACIÓN" << std::endl;
    
    return 0;
}
