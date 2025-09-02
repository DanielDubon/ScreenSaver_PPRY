#!/bin/bash

# Configuración centralizada para el análisis de optimizaciones paralelas

# ============================================================================
# PARÁMETROS DE COMPILACIÓN
# ============================================================================
export CXX="g++"
export CXXFLAGS="-fopenmp -O3 -march=native -std=c++17"
export COMPILER_FLAGS="-fopenmp -O3 -march=native -std=c++17"

# ============================================================================
# PARÁMETROS DE ANÁLISIS
# ============================================================================
export TEST_SIZES="1000 2000 3000 5000"  # Número de círculos
export THREAD_COUNTS="1 2 4 8 16"        # Número de hilos
export SIMULATION_TIME="10.0"             # Tiempo de simulación en segundos
export DELTA_TIME="0.016666667"           # Delta time (1/60 fps)

# ============================================================================
# DIRECTORIOS
# ============================================================================
export DATA_DIR="data"
export CHARTS_DIR="charts"
export UNIFIED_CHARTS_DIR="unified_charts"

# ============================================================================
# ARCHIVOS
# ============================================================================
export UNIFIED_ANALYSIS_CPP="unified_analysis.cpp"
export UNIFIED_ANALYSIS_EXE="unified_analysis"
export UNIFIED_CSV="data/unified_analysis.csv"
export UNIFIED_CHARTS_SCRIPT="generate_unified_charts.py"

export COMPLETE_ANALYSIS_CPP="main_complete_analysis.cpp"
export COMPLETE_ANALYSIS_EXE="complete_analysis"
export COMPLETE_CSV="data/complete_analysis.csv"
export COMPLETE_CHARTS_SCRIPT="generate_charts.py"

# ============================================================================
# FUNCIONES DE UTILIDAD
# ============================================================================

# Función para crear directorios necesarios
create_directories() {
    mkdir -p "$DATA_DIR" "$CHARTS_DIR" "$UNIFIED_CHARTS_DIR"
    echo "✅ Directorios creados: $DATA_DIR, $CHARTS_DIR, $UNIFIED_CHARTS_DIR"
}

# Función para compilar análisis unificado
compile_unified() {
    echo "📦 Compilando análisis unificado..."
    $CXX $CXXFLAGS "$UNIFIED_ANALYSIS_CPP" -o "$UNIFIED_ANALYSIS_EXE"
    if [ $? -eq 0 ]; then
        echo "✅ Compilación exitosa: $UNIFIED_ANALYSIS_EXE"
    else
        echo "❌ Error en la compilación"
        return 1
    fi
}

# Función para compilar análisis completo
compile_complete() {
    echo "📦 Compilando análisis completo..."
    $CXX $CXXFLAGS "$COMPLETE_ANALYSIS_CPP" -o "$COMPLETE_ANALYSIS_EXE"
    if [ $? -eq 0 ]; then
        echo "✅ Compilación exitosa: $COMPLETE_ANALYSIS_EXE"
    else
        echo "❌ Error en la compilación"
        return 1
    fi
}

# Función para ejecutar análisis unificado
run_unified() {
    echo "🔬 Ejecutando análisis unificado..."
    ./"$UNIFIED_ANALYSIS_EXE"
    if [ $? -eq 0 ]; then
        echo "✅ Análisis unificado completado: $UNIFIED_CSV"
    else
        echo "❌ Error en la ejecución"
        return 1
    fi
}

# Función para ejecutar análisis completo
run_complete() {
    echo "🔬 Ejecutando análisis completo..."
    ./"$COMPLETE_ANALYSIS_EXE"
    if [ $? -eq 0 ]; then
        echo "✅ Análisis completo completado: $COMPLETE_CSV"
    else
        echo "❌ Error en la ejecución"
        return 1
    fi
}

# Función para generar gráficas unificadas
generate_unified_charts() {
    echo "📊 Generando gráficas unificadas..."
    python3 "$UNIFIED_CHARTS_SCRIPT"
    if [ $? -eq 0 ]; then
        echo "✅ Gráficas unificadas generadas en: $UNIFIED_CHARTS_DIR"
    else
        echo "❌ Error en la generación de gráficas"
        return 1
    fi
}

# Función para generar gráficas completas
generate_complete_charts() {
    echo "📊 Generando gráficas completas..."
    python3 "$COMPLETE_CHARTS_SCRIPT"
    if [ $? -eq 0 ]; then
        echo "✅ Gráficas completas generadas en: $CHARTS_DIR"
    else
        echo "❌ Error en la generación de gráficas"
        return 1
    fi
}

# Función para mostrar información del sistema
show_system_info() {
    echo "🖥️ Información del sistema:"
    echo "   - Compilador: $CXX"
    echo "   - Flags: $CXXFLAGS"
    echo "   - Hilos disponibles: $(nproc)"
    echo "   - OpenMP disponible: $(gcc -fopenmp -dM -E - < /dev/null | grep -i openmp | wc -l)"
}

# Función para mostrar configuración
show_config() {
    echo "⚙️ Configuración actual:"
    echo "   - Tamaños de prueba: $TEST_SIZES"
    echo "   - Número de hilos: $THREAD_COUNTS"
    echo "   - Tiempo de simulación: $SIMULATION_TIME segundos"
    echo "   - Delta time: $DELTA_TIME"
    echo "   - Directorio de datos: $DATA_DIR"
    echo "   - Directorio de gráficas unificadas: $UNIFIED_CHARTS_DIR"
    echo "   - Directorio de gráficas completas: $CHARTS_DIR"
}

# Mostrar configuración al cargar el archivo
echo "📋 Configuración cargada para análisis de optimizaciones paralelas"
show_config
