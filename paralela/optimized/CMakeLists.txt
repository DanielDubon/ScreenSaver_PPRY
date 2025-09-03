cmake_minimum_required(VERSION 3.16)
project(ScreensaverOptimized VERSION 1.0)

# Configurar C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configurar OpenMP
find_package(OpenMP REQUIRED)

# Detectar el compilador y configurar flags apropiados
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Flags para GCC
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG -ffast-math -funroll-loops -flto -fno-exceptions -fno-rtti -ftree-vectorize")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall -Wextra")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Flags para Clang
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG -ffast-math -funroll-loops -fvectorize -flto -fno-exceptions -fno-rtti")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall -Wextra")
else()
    # Flags genéricos para otros compiladores
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
endif()

# Agregar directorio de headers
include_directories(src)

# Archivos fuente principales
set(SOURCES
    src/main_optimized.cpp
    src/common.h
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
message(STATUS "Compilador detectado: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Ejecutable: ${CMAKE_BINARY_DIR}/bin/main_optimized")

# ==========================================
# VERSIÓN VISUAL OPCIONAL (con SDL)
# ==========================================

# Buscar SDL2 (solo para demo visual)
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(SDL2 QUIET sdl2)
    
    if(SDL2_FOUND)
        message(STATUS "SDL2 encontrado - habilitando demo visual")
        
        # Crear ejecutable de demo visual
        add_executable(visual_demo
            src/main_visual_demo.cpp
            src/common.h
        )
        
        # Configurar SDL2
        target_link_libraries(visual_demo 
            OpenMP::OpenMP_CXX 
            ${SDL2_LIBRARIES}
        )
        target_include_directories(visual_demo PRIVATE ${SDL2_INCLUDE_DIRS})
        target_compile_options(visual_demo PRIVATE ${SDL2_CFLAGS_OTHER})
        
        # Aplicar las mismas optimizaciones que al ejecutable principal
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(visual_demo PRIVATE 
                -O3 -march=native -DNDEBUG -ffast-math -funroll-loops 
                -flto -fno-exceptions -fno-rtti -ftree-vectorize
            )
        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            target_compile_options(visual_demo PRIVATE 
                -O3 -march=native -DNDEBUG -ffast-math -funroll-loops 
                -fvectorize -flto -fno-exceptions -fno-rtti
            )
        else()
            target_compile_options(visual_demo PRIVATE -O3 -DNDEBUG)
        endif()
        
        # Configurar propiedades
        set_target_properties(visual_demo PROPERTIES
            OUTPUT_NAME "visual_demo"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )
        
        # Instalar (opcional)
        install(TARGETS visual_demo
            RUNTIME DESTINATION bin
            OPTIONAL
        )
        
        message(STATUS "Demo visual: ${CMAKE_BINARY_DIR}/bin/visual_demo")
    else()
        message(STATUS "SDL2 no encontrado - solo versión de benchmark disponible")
    endif()
else()
    message(STATUS "pkg-config no disponible - solo versión de benchmark")
endif()