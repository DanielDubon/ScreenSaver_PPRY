# Screensaver paralelo base
cmake_minimum_required(VERSION 3.16)
project(screensaver_omp_base CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

find_package(OpenMP REQUIRED)

add_executable(screensaver_omp_base
     base/src/main_base.cpp
)

target_include_directories(screensaver_omp_base PRIVATE
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(screensaver_omp_base PRIVATE
    ${SDL2_LIBRARIES}
    OpenMP::OpenMP_CXX
)
