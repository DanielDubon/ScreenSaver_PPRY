#!/bin/bash

# Script de ejecución rápida para el análisis unificado

echo "🚀 Ejecutando Análisis Unificado de Optimizaciones Paralelas"
echo "================================================================"

# Verificar que estamos en el directorio correcto
if [ ! -f "unified_analysis.cpp" ]; then
    echo "Error: No se encontró unified_analysis.cpp"
    echo "   Ejecuta este script desde el directorio paralela/optimized/src/"
    exit 1
fi

# Crear directorios si no existen
mkdir -p data unified_charts

echo "Compilando análisis unificado..."
g++ -fopenmp -O3 -march=native -std=c++17 unified_analysis.cpp -o unified_analysis

if [ $? -ne 0 ]; then
    echo "Error en la compilación"
    exit 1
fi

echo "Compilación exitosa"
echo ""

echo "Ejecutando análisis..."
./unified_analysis

if [ $? -ne 0 ]; then
    echo "Error en la ejecución"
    exit 1
fi

echo ""
echo "Generando gráficas..."
python3 generate_unified_charts.py

if [ $? -ne 0 ]; then
    echo "Error en la generación de gráficas"
    exit 1
fi

echo ""
echo "Análisis completado exitosamente!"
echo ""
echo "Archivos generados:"
echo "├── data/unified_analysis.csv     # Datos del análisis"
echo "└── unified_charts/               # Gráficas PNG"
echo "   ├── speedup_comparison.png"
echo "   ├── efficiency_analysis.png"
echo "   ├── scalability_analysis.png"
echo "   ├── performance_summary.png"
echo "   └── thread_analysis.png"
echo ""
echo "Resumen:"
echo "- Implementaciones analizadas: SECUENCIAL, PARALELO_BASE, PARALELO_OPTIMIZADO"
echo "- Configuraciones: 1000, 2000, 3000, 5000 círculos"
echo "- Hilos: 1, 2, 4, 8, 16"
echo "- Optimizaciones integradas: A, B, C, D"
