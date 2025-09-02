cmake_minimum_required(VERSION 3.16)
project(ScreensaverOptimized VERSION 1.0)

# Configurar C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configurar OpenMP
find_package(OpenMP REQUIRED)

# Configurar flags de optimización
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall -Wextra")

# Agregar directorio de headers
include_directories(src)

# Archivos fuente principales
set(SOURCES
    src/main_optimized.cpp
    src/common.h
    src/simulation_base.h
    src/advanced_optimizations.h
    src/data_structures_optimized.h
    src/memory_optimizations.h
)

# Crear ejecutable principal
add_executable(screensaver_optimized ${SOURCES})

# Enlazar OpenMP
target_link_libraries(screensaver_optimized OpenMP::OpenMP_CXX)

# Configurar propiedades del ejecutable
set_target_properties(screensaver_optimized PROPERTIES
    OUTPUT_NAME "main_optimized"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Crear directorios de salida
add_custom_command(TARGET screensaver_optimized POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/data
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/charts
)

# Instalar ejecutable
install(TARGETS screensaver_optimized
    RUNTIME DESTINATION bin
)

# Mensaje de configuración
message(STATUS "Configuración completada para Screensaver Optimized")
message(STATUS "Ejecutable: ${CMAKE_BINARY_DIR}/bin/main_optimized")
