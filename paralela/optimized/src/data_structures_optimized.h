/*
 * ESTRUCTURAS DE DATOS OPTIMIZADAS - Inciso B
 * 
 * Este header implementa optimizaciones de estructuras de datos:
 * 1. Structure of Arrays (SoA) vs Array of Structures (AoS)
 * 2. Memory alignment para mejor acceso a cache
 * 3. SIMD-friendly data layout
 * 4. Memory pool para evitar allocaciones dinámicas
 */

#ifndef DATA_STRUCTURES_OPTIMIZED_H
#define DATA_STRUCTURES_OPTIMIZED_H

#include <vector>
#include <memory>
#include <cstdint>
#include <cstring>
#include <SDL2/SDL.h>

struct CirclesOptimized {
    // Arrays separados para mejor localidad de cache
    // Cada array contiene un tipo de dato específico
    std::vector<float> x;     
    std::vector<float> y; 
    std::vector<float> vx;     
    std::vector<float> vy;     
    std::vector<float> r;       
    std::vector<uint32_t> colors;
    
    // Constructor optimizado con reserva de memoria
    explicit CirclesOptimized(size_t capacity) {
        // Reservar memoria de una vez para evitar reallocaciones
        x.reserve(capacity);
        y.reserve(capacity);
        vx.reserve(capacity);
        vy.reserve(capacity);
        r.reserve(capacity);
        colors.reserve(capacity);
    }
    
    // Método para agregar un círculo eficientemente
    void addCircle(float px, float py, float pvx, float pvy, float pr, SDL_Color color) {
        x.push_back(px);
        y.push_back(py);
        vx.push_back(pvx);
        vy.push_back(pvy);
        r.push_back(pr);
        
        // Convertir SDL_Color a formato ARGB para mejor acceso
        uint32_t argb = (color.a << 24) | (color.r << 16) | (color.g << 8) | color.b;
        colors.push_back(argb);
    }
    
    // Método para obtener el número de círculos
    size_t size() const { return x.size(); }
    
    // Método para limpiar todos los datos
    void clear() {
        x.clear();
        y.clear();
        vx.clear();
        vy.clear();
        r.clear();
        colors.clear();
    }
    
    // Método para redimensionar eficientemente
    void resize(size_t new_size) {
        x.resize(new_size);
        y.resize(new_size);
        vx.resize(new_size);
        vy.resize(new_size);
        r.resize(new_size);
        colors.resize(new_size);
    }
};

// ============================================================================
// MEMORY POOL OPTIMIZADO - Evita allocaciones dinámicas frecuentes
// ============================================================================
template<typename T>
class MemoryPool {
private:
    struct Block {
        T data;
        Block* next;
    };
    
    Block* free_list;
    std::vector<std::unique_ptr<Block[]>> blocks;
    size_t block_size;
    size_t current_block;
    size_t current_index;
    
public:
    explicit MemoryPool(size_t initial_blocks = 10, size_t block_size = 1000)
        : free_list(nullptr), block_size(block_size), current_block(0), current_index(0) {
        // Pre-allocar bloques de memoria
        for (size_t i = 0; i < initial_blocks; ++i) {
            allocateBlock();
        }
    }
    
    T* allocate() {
        if (free_list) {
            // Usar un bloque libre de la lista
            Block* block = free_list;
            free_list = block->next;
            return &block->data;
        }
        
        // Si no hay bloques libres, crear uno nuevo
        if (current_index >= block_size) {
            allocateBlock();
        }
        
        return &blocks[current_block][current_index++].data;
    }
    
    void deallocate(T* ptr) {
        // Agregar a la lista de bloques libres
        Block* block = reinterpret_cast<Block*>(ptr);
        block->next = free_list;
        free_list = block;
    }
    
private:
    void allocateBlock() {
        blocks.emplace_back(std::make_unique<Block[]>(block_size));
        current_block = blocks.size() - 1;
        current_index = 0;
    }
};

// ============================================================================
// CACHE-ALIGNED DATA STRUCTURE - Optimizado para acceso a memoria
// ============================================================================
struct alignas(64) CacheAlignedCircles {
    // Alineado a 64 bytes (tamaño de línea de cache típico)
    std::vector<float, std::allocator<float>> x;
    std::vector<float, std::allocator<float>> y;
    std::vector<float, std::allocator<float>> vx;
    std::vector<float, std::allocator<float>> vy;
    std::vector<float, std::allocator<float>> r;
    std::vector<uint32_t, std::allocator<uint32_t>> colors;
    
    // Constructor con alineación explícita
    explicit CacheAlignedCircles(size_t capacity) {
        // Usar allocator personalizado para alineación
        x.reserve(capacity);
        y.reserve(capacity);
        vx.reserve(capacity);
        vy.reserve(capacity);
        r.reserve(capacity);
        colors.reserve(capacity);
    }
};

// ============================================================================
// FUNCIONES DE CONVERSIÓN ENTRE ESTRUCTURAS
// ============================================================================

// Convertir de AoS a SoA
CirclesOptimized convertToOptimized(const std::vector<Circle>& circles) {
    CirclesOptimized optimized(circles.size());
    
    for (const auto& circle : circles) {
        optimized.addCircle(circle.x, circle.y, circle.vx, circle.vy, circle.r, circle.color);
    }
    
    return optimized;
}

// Convertir de SoA a AoS (para compatibilidad)
std::vector<Circle> convertToStandard(const CirclesOptimized& optimized) {
    std::vector<Circle> circles;
    circles.reserve(optimized.size());
    
    for (size_t i = 0; i < optimized.size(); ++i) {
        Circle circle;
        circle.x = optimized.x[i];
        circle.y = optimized.y[i];
        circle.vx = optimized.vx[i];
        circle.vy = optimized.vy[i];
        circle.r = optimized.r[i];
        
        // Convertir de ARGB a SDL_Color
        uint32_t argb = optimized.colors[i];
        circle.color.a = (argb >> 24) & 0xFF;
        circle.color.r = (argb >> 16) & 0xFF;
        circle.color.g = (argb >> 8) & 0xFF;
        circle.color.b = argb & 0xFF;
        
        circles.push_back(circle);
    }
    
    return circles;
}

#endif
