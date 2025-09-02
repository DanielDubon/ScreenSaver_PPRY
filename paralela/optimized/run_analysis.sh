#!/bin/bash

echo "EJECUTANDO ANALISIS DE OPTIMIZACIONES PARALELAS"
echo "==============================================="

# Verificar que estamos en el directorio correcto
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: No se encontró CMakeLists.txt"
    echo "   Ejecuta este script desde el directorio paralela/optimized/"
    exit 1
fi

# Crear directorio build si no existe
mkdir -p build
cd build

echo "Configurando proyecto con CMake..."
cmake ..

if [ $? -ne 0 ]; then
    echo "Error en la configuración de CMake"
    exit 1
fi

echo "Compilando proyecto..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "Error en la compilación"
    exit 1
fi

echo "Ejecutando análisis..."
./bin/main_optimized

if [ $? -ne 0 ]; then
    echo "Error en la ejecución"
    exit 1
fi

echo "Verificando datos generados..."
if [ -f "data/main_optimized.csv" ]; then
    echo "CSV generado: data/main_optimized.csv"
    echo "Registros: $(wc -l < data/main_optimized.csv)"
else
    echo "Error: No se generó el CSV"
    exit 1
fi

echo ""
echo "Generando gráficas..."
cd ..
python3 src/generate_charts.py

if [ $? -ne 0 ]; then
    echo "Error al generar gráficas"
    exit 1
fi

echo "Verificando gráficas generadas..."
if [ -d "src/charts" ]; then
    echo "Gráficas generadas:"
    ls -la src/charts/*.png
else
    echo "Error: No se generaron las gráficas"
    exit 1
fi

echo ""
echo "RESUMEN DEL ANALISIS:"
echo "====================="
echo "Archivo principal: src/main_optimized.cpp"
echo "Datos generados: build/data/main_optimized.csv"
echo "Gráficas: src/charts/"
echo ""
echo "Gráficas generadas:"
echo "  - openmp_advanced_analysis.png: Cláusulas OpenMP avanzadas"
echo "  - data_structures_analysis.png: Optimización de estructuras de datos"
echo "  - memory_access_analysis.png: Optimización de acceso a memoria"
echo "  - other_mechanisms_analysis.png: Otros mecanismos de optimización"
echo "  - speedup_comparison.png: Comparación Base vs Optimizado"
echo ""
echo "ANALISIS COMPLETADO EXITOSAMENTE!"
