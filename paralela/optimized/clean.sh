#!/bin/bash

echo "LIMPIANDO PROYECTO DE OPTIMIZACIONES PARALELAS"
echo "=============================================="

# Limpiar directorio build
if [ -d "build" ]; then
    echo "Limpiando directorio build..."
    rm -rf build
fi

# Limpiar archivos compilados en src
echo "Limpiando archivos compilados..."
cd src
rm -f *.o *.out main_optimized unified_analysis complete_analysis demo_*

# Limpiar directorios de datos y gráficas
echo "Limpiando directorios de datos..."
rm -rf data charts unified_charts inciso_charts

# Crear directorios vacíos
mkdir -p data charts

echo "Limpieza completada"
echo "Directorio build eliminado"
echo "Archivos compilados eliminados"
echo "Directorios de datos limpiados"
