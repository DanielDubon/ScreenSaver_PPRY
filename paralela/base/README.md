# Screensaver Paralelo (C++/SDL2 + OpenMP)

Animación **paralela** de *N* partículas con rebotes en un lienzo ≥ 640×480.  
Esta versión es el **baseline paralelo**: solo se paraleliza la **fase de update** con OpenMP; el render sigue en el hilo principal con SDL.

## Características
- Paralelización con **OpenMP** (`#pragma omp parallel for`) del movimiento y rebotes.
- Misma apariencia/semántica que el secuencial (comparables 1:1).
- Colores pseudoaleatorios por partícula.
- Control por CLI: `--n`, `--w`, `--h`, `--fps`.
- FPS cap y contador en consola (como referencia).
- Sin extras aún: **no** hay SoA, tiles, grilla, tasks ni SIMD.

---

## Requisitos
- **C++17** y **CMake ≥ 3.16** (opcional si compilas con g++).
- **OpenMP** (soportado por tu compilador, en g++ es `-fopenmp`).
- **SDL2** (headers + libs).

### Linux / WSL
```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config libsdl2-dev
```

> Tip: verifica OpenMP con `g++ --version` (g++ ≥ 9 va bien).

---

## Compilación

### Opción A — CMake (recomendado)
Estructura esperada:
```
paralela/
  ├─ base/
  │  └─ src/main_base.cpp
  └─ CMakeLists.txt
```

Comandos:
```bash
# desde la raíz del repo
cmake -S paralela -B build_base
cmake --build build_base -j
```

Binario resultante:
- Linux/WSL: `./build_base/screensaver_omp_base`

### Opción B — g++ directo (sin CMake)
```bash
g++ -O3 -std=c++17 -fopenmp   paralela/base/src/main_base.cpp   -o screensaver_omp_base   $(pkg-config --cflags --libs sdl2)
```

> Si `pkg-config` no encuentra SDL2, revisa `pkg-config --modversion sdl2` y tu `PKG_CONFIG_PATH`.

---

## Ejecución
```bash
# CMake:
./build_base/screensaver_omp_base --n 2000 --w 1280 --h 720 --fps 60

# g++ directo:
./screensaver_omp_base            --n 2000 --w 1280 --h 720 --fps 60
```

### Control de hilos (OpenMP)
- Por defecto, OpenMP usa todos tus núcleos.  
- También puedes fijarlo por variable de entorno:

```bash
OMP_NUM_THREADS=1 ./build_base/screensaver_omp_base --n 2000 --w 1280 --h 720 --fps 60
OMP_NUM_THREADS=8 ./build_base/screensaver_omp_base --n 2000 --w 1280 --h 720 --fps 60
```

---

## Parámetros CLI
- `--n`   número de partículas (int, por defecto 200).
- `--w`   ancho de ventana (mínimo 640).
- `--h`   alto de ventana (mínimo 480).
- `--fps` límite de FPS (30–240).

Controles:
- `ESC` o cerrar la ventana: salir.

---

## Pruebas rápidas de escalabilidad
- Fija `--n` y ejecuta con distintos `OMP_NUM_THREADS` para ver la mejora.
- Aumenta `--n` (por ejemplo 4000, 8000) para estresar el update y notar el efecto de los hilos.

Ejemplo:
```bash
for t in 1 2 4 8; do
  OMP_NUM_THREADS=$t ./build_base/screensaver_omp_base --n 4000 --w 1280 --h 720 --fps 60
done
```

---

## Notas / Troubleshooting

- **WSL (Windows 11 con WSLg)**: la ventana abre normalmente.  
  En Windows 10 sin WSLg, requiere un servidor X (VcXsrv/Xming) y:
  ```bash
  export DISPLAY=$(grep nameserver /etc/resolv.conf | awk '{print $2}'):0
  export LIBGL_ALWAYS_INDIRECT=1
  ```

- **Renderer**: si hay fallos con el render acelerado, fuerza software sin cambiar código:
  ```bash
  SDL_RENDER_DRIVER=software ./build_base/screensaver_omp_base --n 2000 --w 1280 --h 720 --fps 60
  ```

- **Diferencias respecto al secuencial**: esta versión solo añade paralelismo en `update`.  
  Es la línea base para comparar luego contra la **versión optimizada (extras)**.

---
