# Analisis de Optimizaciones Paralelas - Screensaver

## Descripcion

Este proyecto implementa y analiza optimizaciones paralelas para una simulación de screensaver con círculos en movimiento. Incluye tres implementaciones principales:

1. **SECUENCIAL**: Implementación secuencial pura
2. **PARALELO_BASE**: OpenMP básico con `#pragma omp parallel for`
3. **PARALELO_OPTIMIZADO**: Todas las optimizaciones integradas:
   - Cláusulas OpenMP avanzadas (schedule, reduction, etc.)
   - Optimización de estructuras de datos (SoA, alignment)
   - Optimización de acceso a memoria (prefetching, false sharing)
   - Otros mecanismos (SIMD, task-based, lock-free)

## Estructura del Proyecto

```
paralela/optimized/
├── CMakeLists.txt              # Configuración de CMake
├── run_analysis.sh             # Script de ejecución principal
├── clean.sh                    # Script de limpieza
├── README.md                   # Este archivo
└── src/
    ├── main_optimized.cpp      # Código principal
    ├── generate_charts.py      # Generador de gráficas
    ├── common.h                # Estructuras básicas
    ├── simulation_base.h       # Implementación paralelo base
    ├── advanced_optimizations.h # Optimizaciones avanzadas
    ├── data_structures_optimized.h # Optimización de estructuras
    ├── memory_optimizations.h  # Optimización de memoria
    ├── data/                   # Datos CSV generados
    └── charts/                 # Gráficas generadas
```

## Requisitos

- **Compilador**: GCC con soporte para C++17
- **OpenMP**: Biblioteca de paralelización
- **Python 3**: Para generar gráficas
- **Librerías Python**: pandas, matplotlib, seaborn, numpy
- **CMake**: Sistema de construcción

## Instalación de Dependencias

```bash
# Instalar librerías Python
pip3 install pandas matplotlib seaborn numpy

# Verificar OpenMP
gcc --version
echo $OMP_NUM_THREADS
```

## Uso

### Ejecución Completa

```bash
# Desde el directorio paralela/optimized/
chmod +x run_analysis.sh
./run_analysis.sh
```

### Ejecución Manual

```bash
# 1. Configurar y compilar
mkdir -p build
cd build
cmake ..
make -j$(nproc)

# 2. Ejecutar análisis
./bin/main_optimized

# 3. Generar gráficas
cd ..
python3 src/generate_charts.py
```

### Limpieza

```bash
chmod +x clean.sh
./clean.sh
```

## Resultados

### Datos Generados

- **CSV**: `build/data/main_optimized.csv` - Datos de rendimiento
- **Registros**: ~37 registros con métricas de speedup y eficiencia

### Gráficas Generadas

- **openmp_advanced_analysis.png**: Cláusulas OpenMP avanzadas
- **data_structures_analysis.png**: Optimización de estructuras de datos
- **memory_access_analysis.png**: Optimización de acceso a memoria
- **other_mechanisms_analysis.png**: Otros mecanismos de optimización
- **speedup_comparison.png**: Comparación Base vs Optimizado

## Configuración

### Parámetros de Análisis

- **Tamaños de prueba**: 1000, 2000, 3000, 5000 círculos
- **Número de hilos**: 1, 2, 4, 8, 16
- **Tiempo de simulación**: 10 segundos
- **Delta time**: 1/60 segundos

### Optimizaciones Implementadas

1. **Cláusulas OpenMP Avanzadas**:
   - `schedule(dynamic, 64)`: Distribución dinámica de trabajo
   - `reduction(+:bounces,total_energy)`: Reducción de variables
   - `private(elapsed_time)`: Variables privadas por hilo
   - `shared(circles, width, height, delta_time)`: Variables compartidas

2. **Optimización de Estructuras de Datos**:
   - SoA (Structure of Arrays) en lugar de AoS
   - Alineación de memoria para SIMD
   - Reserva de capacidad en vectores

3. **Optimización de Acceso a Memoria**:
   - Prefetching con `__builtin_prefetch`
   - Acceso SIMD-friendly a arrays
   - Evitar false sharing

4. **Otros Mecanismos**:
   - Optimizaciones de compilador (-O3, -march=native)
   - SIMD vectorization automática
   - Task-based parallelism

## Métricas Analizadas

- **Speedup**: Mejora de rendimiento vs implementación secuencial
- **Eficiencia**: Porcentaje de utilización de hilos
- **Tiempo de ejecución**: Tiempo total de simulación
- **Iteraciones**: Número de pasos de simulación completados

## Scripts Disponibles

### run_analysis.sh
Script principal que ejecuta todo el análisis:
- Configura el proyecto con CMake
- Compila el código
- Ejecuta el análisis
- Genera las gráficas

### clean.sh
Script de limpieza que elimina:
- Directorio build
- Archivos compilados
- Datos y gráficas generadas

## Troubleshooting

### Error de Compilación
```bash
# Verificar OpenMP
gcc -fopenmp -dM -E - < /dev/null | grep -i openmp

# Verificar C++17
gcc --version
```

### Error de Python
```bash
# Instalar dependencias
pip3 install pandas matplotlib seaborn numpy

# Verificar instalación
python3 -c "import pandas, matplotlib, seaborn, numpy; print('OK')"
```

### Error de CMake
```bash
# Instalar CMake
sudo apt-get install cmake

# Verificar versión
cmake --version
```
