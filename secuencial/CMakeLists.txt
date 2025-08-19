cmake_minimum_required(VERSION 3.14)
project(screensaver_seq LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

# SDL2
pkg_check_modules(SDL2 REQUIRED sdl2)
# SDL2_ttf
pkg_check_modules(SDL2TTF REQUIRED SDL2_ttf)

add_executable(screensaver src/main.cpp)

# Includes
target_include_directories(screensaver PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${SDL2TTF_INCLUDE_DIRS}
)

# Librerias
target_link_libraries(screensaver
    ${SDL2_LIBRARIES}
    ${SDL2TTF_LIBRARIES}
)

# Flags extra
target_compile_options(screensaver PRIVATE
    ${SDL2_CFLAGS_OTHER}
    ${SDL2TTF_CFLAGS_OTHER}
)
