#!/bin/bash
echo "EJECUTANDO ANALISIS DE OPTIMIZACIONES PARALELAS"
echo "==============================================="

# Compilar versión de benchmark
echo "Compilando versión de benchmark..."
cmake --build build --target screensaver_optimized

# Ejecutar análisis de rendimiento
echo "Ejecutando análisis..."
./build/bin/main_optimized

echo "Generando gráficas..."
python3 src/generate_charts.py

# Opcionalmente compilar y mostrar demo visual
read -p "¿Ejecutar demostración visual? (y/N): " -n 1 -r
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo -e "\nCompilando versión visual..."
    cmake --build build --target visual_demo
    echo "Ejecutando demostración (presiona ESC para salir)..."
    ./build/bin/visual_demo --n 800 --threads 8
fi

echo "Análisis completado!"