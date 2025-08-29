/*
 * Screensaver Paralelo con Optimizaciones OpenMP Avanzadas
 * 
 * OPTIMIZACIONES IMPLEMENTADAS (Inciso A):
 * 1. collapse(2): Paralelización de bucles anidados en drawFilledCircle
 * 2. atomic: Contadores de rebotes sin condiciones de carrera
 * 3. sections: Tareas independientes paralelas
 * 4. reduction: Cálculos paralelos con reducción automática
 * 5. firstprivate: Variables privadas inicializadas
 * 
 * CRITERIO DE SELECCIÓN DE CLAÚSULAS:
 * 
 * COLLAPSE(2): Se usa en drawFilledCircle porque tenemos bucles anidados (dy y dx).
 *               Sin collapse, solo el bucle externo se paraleliza, creando desbalance
 *               de carga. Con collapse(2), OpenMP combina ambos bucles en uno solo
 *               para mejor distribución del trabajo entre hilos.
 * 
 * ATOMIC: Se usa para incrementar totalBounces porque múltiples hilos pueden
 *          detectar colisiones simultáneamente. Sin atomic, habría race conditions.
 *          Es más eficiente que critical porque no bloquea otros hilos.
 * 
 * SECTIONS: Se usa para ejecutar tareas independientes en paralelo:
 *            - Renderizar círculos
 *            - Calcular estadísticas
 *           Esto aprovecha el paralelismo a nivel de tarea, no solo de datos.
 * 
 * FIRSTPRIVATE: Se usa para dt en el bucle de movimiento porque cada hilo
 *               necesita su propia copia inicializada con 0.016f. Es más eficiente
 *               que private porque evita copiar el valor en cada iteración.
 * 
 * REDUCTION: Se usa para kinetic y potential porque cada hilo suma localmente
 *            y OpenMP combina automáticamente al final, evitando race conditions.
 * 
 * Estas optimizaciones demuestran el uso de cláusulas OpenMP
 * no vistas en clase para mejorar el rendimiento paralelo.
 */
 # Screensaver Paralelo Optimizado (OpenMP Avanzado)

## Descripción
Esta es la versión **OPTIMIZADA** del screensaver paralelo que implementa cláusulas y directivas de OpenMP **no vistas en clase** para el inciso a.

## Optimizaciones Implementadas

### 1. `collapse(2)` - Paralelización de bucles anidados
- **Ubicación**: Función `drawFilledCircle`
- **Beneficio**: Mejor distribución del trabajo de renderizado entre hilos
- **Código**: `#pragma omp parallel for collapse(2) schedule(dynamic, 4)`

### 2. `atomic` - Operaciones atómicas
- **Ubicación**: Contador de rebotes en el bucle principal
- **Beneficio**: Evita condiciones de carrera sin usar `critical` (más eficiente)
- **Código**: `#pragma omp atomic` para `totalBounces++`

### 3. `sections` - Tareas independientes
- **Ubicación**: Renderizado y cálculos de estadísticas
- **Beneficio**: Permite ejecutar tareas diferentes en paralelo
- **Código**: `#pragma omp parallel sections`

### 4. `reduction` - Cálculos paralelos con reducción
- **Ubicación**: Cálculo de velocidad total y círculos rápidos
- **Beneficio**: Automatiza la reducción de variables en paralelo
- **Código**: `#pragma omp parallel for reduction(+:totalVel, fastCircles)`

### 5. `firstprivate` - Variables privadas inicializadas
- **Ubicación**: Bucle de movimiento de círculos
- **Beneficio**: Cada hilo tiene su copia inicializada de `dt`
- **Código**: `#pragma omp parallel for schedule(dynamic, 256) firstprivate(dt)`

## Compilación

### Opción 1: Usando CMake (recomendado)
```bash
# Desde la raíz del proyecto
cmake -S paralela -B build_optimized -f paralela/CMakeLists_optimized.txt
cmake --build build_optimized -j
```

### Opción 2: Compilación directa con g++
```bash
g++ -O3 -std=c++17 -fopenmp -march=native \
    paralela/base/src/main_optimized.cpp \
    -o screensaver_omp_optimized \
    $(pkg-config --cflags --libs sdl2)
```

## Ejecución

### Comandos básicos
```bash
# Versión optimizada
./build_optimized/screensaver_omp_optimized --n 500 --w 1024 --h 768 --fps 60

# Con render por software (si hay problemas)
SDL_RENDER_DRIVER=software ./build_optimized/screensaver_omp_optimized --n 500 --w 1024 --h 768 --fps 60
```

### Control de hilos OpenMP
```bash
# Fijar número de hilos
OMP_NUM_THREADS=4 ./build_optimized/screensaver_omp_optimized --n 500 --w 1024 --h 768 --fps 60
OMP_NUM_THREADS=8 ./build_optimized/screensaver_omp_optimized --n 500 --w 1024 --h 768 --fps 60
```

## Comparación de Performance

### Para demostrar el speedup del inciso a:
1. **Compilar la versión base** (sin optimizaciones avanzadas)
2. **Compilar esta versión optimizada**
3. **Comparar ambas con la misma carga de trabajo**

### Ejemplo de comparación:
```bash
# Versión base (sin optimizaciones avanzadas)
./build_base/screensaver_omp_base --n 1000 --w 1280 --h 720 --fps 60

# Versión optimizada (con cláusulas avanzadas)
./build_optimized/screensaver_omp_optimized --n 1000 --w 1280 --h 720 --fps 60
```

## Diferencias con la versión base

- **Funcionalidades adicionales**: Contador de rebotes, estadísticas en tiempo real
- **Paralelización más granular**: Movimiento + renderizado + estadísticas
- **Cláusulas OpenMP avanzadas**: `collapse`, `atomic`, `sections`, `reduction`
- **Mejor distribución del trabajo**: Especialmente en el renderizado

## Notas importantes

- Esta versión puede tener FPS más bajo debido a las funcionalidades adicionales
- El speedup real se demuestra en cargas de trabajo más pesadas
- Las optimizaciones están diseñadas para mejorar la escalabilidad, no necesariamente el FPS base
