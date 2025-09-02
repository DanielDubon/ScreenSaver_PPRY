#!/bin/bash

echo "EJECUTANDO ANÁLISIS POR INCISOS - OPTIMIZACIONES A, B, C, D"
echo "================================================================"

# Verificar que estamos en el directorio correcto
if [ ! -f "main_optimized.cpp" ]; then
    echo "Error: No se encontró main_optimized.cpp"
    echo "   Ejecuta este script desde el directorio paralela/optimized/src/"
    exit 1
fi

# Crear directorios si no existen
mkdir -p data inciso_charts

echo "Compilando análisis optimizado..."
g++ -fopenmp -O3 -march=native -std=c++17 main_optimized.cpp -o main_optimized

if [ $? -ne 0 ]; then
    echo "Error en la compilación"
    exit 1
fi

echo "Compilación exitosa"
echo ""

echo "Ejecutando análisis por incisos..."
./main_optimized

if [ $? -ne 0 ]; then
    echo "Error en la ejecución"
    exit 1
fi

echo "Análisis completado"
echo ""

echo "Verificando datos generados..."
if [ -f "data/main_optimized.csv" ]; then
    echo "CSV generado: data/main_optimized.csv"
    echo "Registros: $(wc -l < data/main_optimized.csv)"
else
    echo "Error: No se generó el CSV"
    exit 1
fi

echo ""
echo "Generando gráficas por inciso..."
python3 generate_charts.py

if [ $? -ne 0 ]; then
    echo "Error al generar gráficas"
    exit 1
fi

echo "Gráficas generadas"
echo ""

echo "Verificando gráficas generadas..."
if [ -d "inciso_charts" ]; then
    echo "Directorio: inciso_charts/"
    echo "Gráficas generadas:"
    ls -la inciso_charts/*.png
else
    echo "Error: No se generaron las gráficas"
    exit 1
fi

echo ""
echo "RESUMEN DEL ANÁLISIS POR INCISOS:"
echo "====================================="
echo "Archivo principal: main_optimized.cpp"
echo "Datos generados: data/main_optimized.csv"
echo "Gráficas por inciso: inciso_charts/"
echo ""
echo "Gráficas generadas:"
echo "  - inciso_a_openmp_avanzado.png: Cláusulas OpenMP avanzadas"
echo "  - inciso_b_estructuras_datos.png: Optimización de estructuras de datos"
echo "  - inciso_c_acceso_memoria.png: Optimización de acceso a memoria"
echo "  - inciso_d_otros_mecanismos.png: Otros mecanismos de optimización"
echo "  - speedup_base_vs_optimizado.png: Comparación Base vs Optimizado"
echo ""
echo "ANÁLISIS POR INCISOS COMPLETADO EXITOSAMENTE!"
