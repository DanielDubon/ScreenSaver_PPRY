# Screensaver Secuencial (C++/SDL2)

Animacióo secuencial de *N* particulas con rebotes en un lienzo ≥ 640×480. Esta versión sirve como **base** para luego paralelizar (OpenMP) y medir *speedup*.

## Caracteristicas
- Movimiento con fisica simple.
- Colores pseudoaleatorios por partícula.
- Control por CLI: `--n`, `--w`, `--h`, `--fps`.
- FPS “cap” y contador en consola.
- Sin dependencias raras (solo **SDL2**).

---

## Requisitos
- **C++17** y **CMake ≥ 3.14** (opcional si compilas con g++).
- **SDL2** (headers + libs).

### Linux
```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config libsdl2-dev
sudo apt install libsdl2-ttf-dev

```

## Compilacion

### Opcion A — CMake

```bash
cmake -S . -B build
cmake --build build -j
```
Binario resultante:
- Linux: `./build/screensaver`

### Opcion B — g++ directo
```bash
g++ src/main.cpp -O2 -std=c++17 $(pkg-config --cflags --libs sdl2) -o screensaver
```

> Si `pkg-config` no encuentra SDL2, verifica `pkg-config --modversion sdl2` y tu `PKG_CONFIG_PATH`.

---

## Ejecucion
```bash
./build/screensaver --n 300 --w 1024 --h 768 --fps 60
# o si usaste g++ directo:
./screensaver --n 300 --w 1024 --h 768 --fps 60
```

### Controles
- `ESC` o cerrar la ventana: salir.

### Parámetros CLI
- `--n`   número de partículas (int, por defecto 200).
- `--w`   ancho de ventana (mínimo 640).
- `--h`   alto de ventana (mínimo 480).
- `--fps` límite de FPS (30–240).

---

## Si la ventana se cierra o hace *crash* (WSL/Wayland/NVIDIA)
Algunos drivers fallan con el **renderer acelerado** de SDL. Forza render **por software** sin tocar el código:

```bash
SDL_RENDER_DRIVER=software ./screensaver --n 300 --w 1024 --h 768 --fps 60
```
