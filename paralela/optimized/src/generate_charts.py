#!/usr/bin/env python3
"""
GENERADOR DE GRAFICAS PARA ANALISIS DE OPTIMIZACIONES PARALELAS
=============================================================

Este script genera gráficas específicas para el análisis de optimizaciones:
- Cláusulas OpenMP avanzadas
- Optimización de estructuras de datos
- Optimización de acceso a memoria
- Otros mecanismos de optimización
- Comparación Base vs Optimizado
"""

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
from matplotlib import rcParams
import sys
import os

# Configurar estilo profesional
plt.style.use('seaborn-v0_8')
rcParams['font.family'] = 'DejaVu Sans'
rcParams['font.size'] = 11
rcParams['axes.titlesize'] = 14
rcParams['axes.labelsize'] = 12
rcParams['xtick.labelsize'] = 10
rcParams['ytick.labelsize'] = 10
rcParams['legend.fontsize'] = 10

def load_data(csv_file):
    """Cargar datos del archivo CSV"""
    try:
        df = pd.read_csv(csv_file)
        print(f"Datos cargados: {len(df)} registros")
        print(f"Implementaciones: {df['Implementation'].unique()}")
        return df
    except FileNotFoundError:
        print(f"Error: No se encontró el archivo {csv_file}")
        return None
    except Exception as e:
        print(f"Error al cargar datos: {e}")
        return None

def create_openmp_advanced_chart(df, output_file="openmp_advanced_analysis.png"):
    """Gráfica para cláusulas OpenMP avanzadas"""
    plt.figure(figsize=(12, 8))
    
    # Filtrar solo implementaciones paralelas
    parallel_df = df[df['Implementation'] != 'SECUENCIAL']
    
    # Crear subplots
    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(16, 12))
    
    # 1. Speedup por número de hilos
    speedup_by_threads = parallel_df.groupby(['NumThreads', 'Implementation'])['Speedup'].mean().unstack()
    speedup_by_threads.plot(kind='line', marker='o', ax=ax1, linewidth=3, markersize=8)
    ax1.set_xlabel('Número de Hilos')
    ax1.set_ylabel('Speedup')
    ax1.set_title('Cláusulas OpenMP Avanzadas\nSpeedup vs Número de Hilos')
    ax1.grid(True, alpha=0.3)
    ax1.legend(title='Implementación')
    
    # 2. Eficiencia por número de hilos
    efficiency_by_threads = parallel_df.groupby(['NumThreads', 'Implementation'])['Efficiency'].mean().unstack()
    efficiency_by_threads.plot(kind='line', marker='s', ax=ax2, linewidth=3, markersize=8)
    ax2.set_xlabel('Número de Hilos')
    ax2.set_ylabel('Eficiencia (%)')
    ax2.set_title('Cláusulas OpenMP Avanzadas\nEficiencia vs Número de Hilos')
    ax2.grid(True, alpha=0.3)
    ax2.legend(title='Implementación')
    
    # 3. Speedup por número de círculos
    speedup_by_circles = parallel_df.groupby(['NumCircles', 'Implementation'])['Speedup'].mean().unstack()
    speedup_by_circles.plot(kind='bar', ax=ax3, alpha=0.8)
    ax3.set_xlabel('Número de Círculos')
    ax3.set_ylabel('Speedup')
    ax3.set_title('Cláusulas OpenMP Avanzadas\nSpeedup vs Número de Círculos')
    ax3.grid(True, alpha=0.3)
    ax3.legend(title='Implementación')
    ax3.tick_params(axis='x', rotation=45)
    
    # 4. Comparación de tiempo de ejecución
    time_comparison = parallel_df.groupby('Implementation')['ExecutionTime'].mean().sort_values(ascending=False)
    bars = ax4.bar(time_comparison.index, time_comparison.values, 
                   color=['#1f77b4', '#ff7f0e'], alpha=0.8)
    ax4.set_ylabel('Tiempo de Ejecución Promedio (s)')
    ax4.set_title('Cláusulas OpenMP Avanzadas\nTiempo de Ejecución por Implementación')
    ax4.grid(True, alpha=0.3)
    ax4.tick_params(axis='x', rotation=45)
    
    # Agregar valores en las barras
    for bar, value in zip(bars, time_comparison.values):
        height = bar.get_height()
        ax4.text(bar.get_x() + bar.get_width()/2., height + 0.01,
                f'{value:.3f}s', ha='center', va='bottom')
    
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Gráfica OpenMP avanzado guardada: {output_file}")
    plt.close()

def create_data_structures_chart(df, output_file="data_structures_analysis.png"):
    """Gráfica para optimización de estructuras de datos"""
    plt.figure(figsize=(12, 8))
    
    # Filtrar solo implementaciones paralelas
    parallel_df = df[df['Implementation'] != 'SECUENCIAL']
    
    # Crear subplots
    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(16, 12))
    
    # 1. Mejora de rendimiento por tamaño de datos
    improvement_by_size = []
    circle_sizes = sorted(parallel_df['NumCircles'].unique())
    
    for size in circle_sizes:
        size_data = parallel_df[parallel_df['NumCircles'] == size]
        base_avg = size_data[size_data['Implementation'] == 'PARALELO_BASE']['Speedup'].mean()
        opt_avg = size_data[size_data['Implementation'] == 'PARALELO_OPTIMIZADO']['Speedup'].mean()
        improvement = ((opt_avg - base_avg) / base_avg) * 100 if base_avg > 0 else 0
        improvement_by_size.append(improvement)
    
    ax1.bar(circle_sizes, improvement_by_size, color='#2ca02c', alpha=0.8)
    ax1.set_xlabel('Número de Círculos')
    ax1.set_ylabel('Mejora de Rendimiento (%)')
    ax1.set_title('Optimización de Estructuras de Datos\nMejora vs Tamaño de Datos')
    ax1.grid(True, alpha=0.3)
    
    # Agregar valores en las barras
    for i, v in enumerate(improvement_by_size):
        ax1.text(circle_sizes[i], v + 0.5, f'{v:.1f}%', ha='center', va='bottom')
    
    # 2. Eficiencia de memoria por implementación
    efficiency_comparison = parallel_df.groupby('Implementation')['Efficiency'].mean().sort_values(ascending=True)
    bars = ax2.barh(efficiency_comparison.index, efficiency_comparison.values, 
                    color=['#1f77b4', '#ff7f0e'], alpha=0.8)
    ax2.set_xlabel('Eficiencia Promedio (%)')
    ax2.set_title('Optimización de Estructuras de Datos\nEficiencia por Implementación')
    ax2.grid(True, alpha=0.3)
    
    # Agregar valores en las barras
    for bar, value in zip(bars, efficiency_comparison.values):
        width = bar.get_width()
        ax2.text(width + 0.5, bar.get_y() + bar.get_height()/2.,
                f'{value:.1f}%', ha='left', va='center')
    
    # 3. Escalabilidad por número de hilos
    scalability_data = parallel_df.groupby(['NumThreads', 'Implementation'])['Speedup'].mean().unstack()
    scalability_data.plot(kind='line', marker='o', ax=ax3, linewidth=3, markersize=8)
    ax3.set_xlabel('Número de Hilos')
    ax3.set_ylabel('Speedup')
    ax3.set_title('Optimización de Estructuras de Datos\nEscalabilidad vs Número de Hilos')
    ax3.grid(True, alpha=0.3)
    ax3.legend(title='Implementación')
    
    # 4. Comparación de rendimiento por configuración
    pivot_data = parallel_df.pivot_table(
        index='NumCircles', 
        columns='Implementation', 
        values='Speedup', 
        aggfunc='mean'
    )
    pivot_data.plot(kind='bar', ax=ax4, alpha=0.8)
    ax4.set_xlabel('Número de Círculos')
    ax4.set_ylabel('Speedup Promedio')
    ax4.set_title('Optimización de Estructuras de Datos\nRendimiento por Configuración')
    ax4.grid(True, alpha=0.3)
    ax4.legend(title='Implementación')
    ax4.tick_params(axis='x', rotation=45)
    
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Gráfica estructuras de datos guardada: {output_file}")
    plt.close()

def create_memory_access_chart(df, output_file="memory_access_analysis.png"):
    """Gráfica para optimización de acceso a memoria"""
    plt.figure(figsize=(12, 8))
    
    # Filtrar solo implementaciones paralelas
    parallel_df = df[df['Implementation'] != 'SECUENCIAL']
    
    # Crear subplots
    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(16, 12))
    
    # 1. Análisis de cache performance
    cache_performance = parallel_df.groupby(['NumThreads', 'Implementation'])['ExecutionTime'].mean().unstack()
    cache_performance.plot(kind='line', marker='s', ax=ax1, linewidth=3, markersize=8)
    ax1.set_xlabel('Número de Hilos')
    ax1.set_ylabel('Tiempo de Ejecución (s)')
    ax1.set_title('Optimización de Acceso a Memoria\nCache Performance vs Número de Hilos')
    ax1.grid(True, alpha=0.3)
    ax1.legend(title='Implementación')
    
    # 2. Mejora de rendimiento por hilos
    improvement_by_threads = []
    thread_counts = sorted(parallel_df['NumThreads'].unique())
    
    for threads in thread_counts:
        thread_data = parallel_df[parallel_df['NumThreads'] == threads]
        base_avg = thread_data[thread_data['Implementation'] == 'PARALELO_BASE']['Speedup'].mean()
        opt_avg = thread_data[thread_data['Implementation'] == 'PARALELO_OPTIMIZADO']['Speedup'].mean()
        improvement = ((opt_avg - base_avg) / base_avg) * 100 if base_avg > 0 else 0
        improvement_by_threads.append(improvement)
    
    ax2.bar(thread_counts, improvement_by_threads, color='#d62728', alpha=0.8)
    ax2.set_xlabel('Número de Hilos')
    ax2.set_ylabel('Mejora de Rendimiento (%)')
    ax2.set_title('Optimización de Acceso a Memoria\nMejora vs Número de Hilos')
    ax2.grid(True, alpha=0.3)
    
    # Agregar valores en las barras
    for i, v in enumerate(improvement_by_threads):
        ax2.text(thread_counts[i], v + 0.5, f'{v:.1f}%', ha='center', va='bottom')
    
    # 3. Eficiencia de memoria por tamaño de datos
    memory_efficiency = parallel_df.groupby(['NumCircles', 'Implementation'])['Efficiency'].mean().unstack()
    memory_efficiency.plot(kind='bar', ax=ax3, alpha=0.8)
    ax3.set_xlabel('Número de Círculos')
    ax3.set_ylabel('Eficiencia (%)')
    ax3.set_title('Optimización de Acceso a Memoria\nEficiencia vs Tamaño de Datos')
    ax3.grid(True, alpha=0.3)
    ax3.legend(title='Implementación')
    ax3.tick_params(axis='x', rotation=45)
    
    # 4. Comparación de throughput
    throughput_data = parallel_df.groupby('Implementation')['Speedup'].mean().sort_values(ascending=True)
    bars = ax4.barh(throughput_data.index, throughput_data.values, 
                    color=['#1f77b4', '#ff7f0e'], alpha=0.8)
    ax4.set_xlabel('Speedup Promedio')
    ax4.set_title('Optimización de Acceso a Memoria\nThroughput por Implementación')
    ax4.grid(True, alpha=0.3)
    
    # Agregar valores en las barras
    for bar, value in zip(bars, throughput_data.values):
        width = bar.get_width()
        ax4.text(width + 0.01, bar.get_y() + bar.get_height()/2.,
                f'{value:.2f}x', ha='left', va='center')
    
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Gráfica acceso a memoria guardada: {output_file}")
    plt.close()

def create_other_mechanisms_chart(df, output_file="other_mechanisms_analysis.png"):
    """Gráfica para otros mecanismos de optimización"""
    plt.figure(figsize=(12, 8))
    
    # Filtrar solo implementaciones paralelas
    parallel_df = df[df['Implementation'] != 'SECUENCIAL']
    
    # Crear subplots
    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(16, 12))
    
    # 1. Análisis de optimizaciones avanzadas
    advanced_analysis = parallel_df.groupby(['NumThreads', 'Implementation'])['Speedup'].mean().unstack()
    advanced_analysis.plot(kind='line', marker='^', ax=ax1, linewidth=3, markersize=8)
    ax1.set_xlabel('Número de Hilos')
    ax1.set_ylabel('Speedup')
    ax1.set_title('Otros Mecanismos de Optimización\nSpeedup vs Número de Hilos')
    ax1.grid(True, alpha=0.3)
    ax1.legend(title='Implementación')
    
    # 2. Eficiencia de optimizaciones
    optimization_efficiency = parallel_df.groupby(['NumCircles', 'Implementation'])['Efficiency'].mean().unstack()
    optimization_efficiency.plot(kind='line', marker='o', ax=ax2, linewidth=3, markersize=8)
    ax2.set_xlabel('Número de Círculos')
    ax2.set_ylabel('Eficiencia (%)')
    ax2.set_title('Otros Mecanismos de Optimización\nEficiencia vs Tamaño de Datos')
    ax2.grid(True, alpha=0.3)
    ax2.legend(title='Implementación')
    
    # 3. Comparación de rendimiento máximo
    max_performance = parallel_df.groupby('Implementation')['Speedup'].max().sort_values(ascending=True)
    bars = ax3.barh(max_performance.index, max_performance.values, 
                    color=['#1f77b4', '#ff7f0e'], alpha=0.8)
    ax3.set_xlabel('Speedup Máximo')
    ax3.set_title('Otros Mecanismos de Optimización\nRendimiento Máximo por Implementación')
    ax3.grid(True, alpha=0.3)
    
    # Agregar valores en las barras
    for bar, value in zip(bars, max_performance.values):
        width = bar.get_width()
        ax3.text(width + 0.01, bar.get_y() + bar.get_height()/2.,
                f'{value:.2f}x', ha='left', va='center')
    
    # 4. Análisis de escalabilidad
    scalability_analysis = parallel_df.pivot_table(
        index='NumThreads', 
        columns='NumCircles', 
        values='Speedup', 
        aggfunc='mean'
    )
    sns.heatmap(scalability_analysis, annot=True, fmt='.2f', cmap='YlOrRd', 
                cbar_kws={'label': 'Speedup'}, ax=ax4)
    ax4.set_title('Otros Mecanismos de Optimización\nMapa de Calor: Escalabilidad')
    ax4.set_xlabel('Número de Círculos')
    ax4.set_ylabel('Número de Hilos')
    
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Gráfica otros mecanismos guardada: {output_file}")
    plt.close()

def create_speedup_comparison_chart(df, output_file="speedup_comparison.png"):
    """Gráfica de comparación de speedup: Base vs Optimizado"""
    plt.figure(figsize=(14, 10))
    
    # Filtrar solo implementaciones paralelas
    parallel_df = df[df['Implementation'] != 'SECUENCIAL']
    
    # Crear subplots
    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(16, 12))
    
    # 1. Comparación general de speedup
    speedup_comparison = parallel_df.groupby('Implementation')['Speedup'].mean().sort_values(ascending=True)
    bars = ax1.barh(speedup_comparison.index, speedup_comparison.values, 
                    color=['#1f77b4', '#ff7f0e'], alpha=0.8)
    ax1.set_xlabel('Speedup Promedio')
    ax1.set_title('Comparación General: Base vs Optimizado\nSpeedup Promedio')
    ax1.grid(True, alpha=0.3)
    
    # Agregar valores en las barras
    for bar, value in zip(bars, speedup_comparison.values):
        width = bar.get_width()
        ax1.text(width + 0.01, bar.get_y() + bar.get_height()/2.,
                f'{value:.2f}x', ha='left', va='center')
    
    # 2. Speedup por número de hilos
    speedup_by_threads = parallel_df.groupby(['NumThreads', 'Implementation'])['Speedup'].mean().unstack()
    speedup_by_threads.plot(kind='line', marker='o', ax=ax2, linewidth=3, markersize=8)
    ax2.set_xlabel('Número de Hilos')
    ax2.set_ylabel('Speedup')
    ax2.set_title('Speedup vs Número de Hilos\nBase vs Optimizado')
    ax2.grid(True, alpha=0.3)
    ax2.legend(title='Implementación')
    
    # 3. Speedup por número de círculos
    speedup_by_circles = parallel_df.groupby(['NumCircles', 'Implementation'])['Speedup'].mean().unstack()
    speedup_by_circles.plot(kind='bar', ax=ax3, alpha=0.8)
    ax3.set_xlabel('Número de Círculos')
    ax3.set_ylabel('Speedup')
    ax3.set_title('Speedup vs Número de Círculos\nBase vs Optimizado')
    ax3.grid(True, alpha=0.3)
    ax3.legend(title='Implementación')
    ax3.tick_params(axis='x', rotation=45)
    
    # 4. Mejora porcentual
    base_avg = parallel_df[parallel_df['Implementation'] == 'PARALELO_BASE']['Speedup'].mean()
    opt_avg = parallel_df[parallel_df['Implementation'] == 'PARALELO_OPTIMIZADO']['Speedup'].mean()
    improvement = ((opt_avg - base_avg) / base_avg) * 100
    
    labels = ['Paralelo Base', 'Paralelo Optimizado']
    values = [base_avg, opt_avg]
    colors = ['#1f77b4', '#ff7f0e']
    
    bars = ax4.bar(labels, values, color=colors, alpha=0.8)
    ax4.set_ylabel('Speedup Promedio')
    ax4.set_title(f'Comparación Final: Base vs Optimizado\nMejora: {improvement:.1f}%')
    ax4.grid(True, alpha=0.3)
    
    # Agregar valores en las barras
    for bar, value in zip(bars, values):
        height = bar.get_height()
        ax4.text(bar.get_x() + bar.get_width()/2., height + 0.01,
                f'{value:.2f}x', ha='center', va='bottom')
    
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Gráfica de comparación de speedup guardada: {output_file}")
    plt.close()

def main():
    """Función principal"""
    csv_file = "data/main_optimized.csv"
    
    if len(sys.argv) > 1:
        csv_file = sys.argv[1]
    
    print("GENERADOR DE GRAFICAS PARA ANALISIS DE OPTIMIZACIONES PARALELAS")
    print("=============================================================")
    print(f"Archivo CSV: {csv_file}")
    print()
    
    # Cargar datos
    df = load_data(csv_file)
    if df is None:
        return
    
    # Crear directorio para gráficas si no existe
    charts_dir = "charts"
    if not os.path.exists(charts_dir):
        os.makedirs(charts_dir)
    
    # Generar gráficas
    print("\nGenerando gráficas...")
    
    create_openmp_advanced_chart(df, f"{charts_dir}/openmp_advanced_analysis.png")
    create_data_structures_chart(df, f"{charts_dir}/data_structures_analysis.png")
    create_memory_access_chart(df, f"{charts_dir}/memory_access_analysis.png")
    create_other_mechanisms_chart(df, f"{charts_dir}/other_mechanisms_analysis.png")
    create_speedup_comparison_chart(df, f"{charts_dir}/speedup_comparison.png")
    
    print(f"\nTodas las gráficas han sido guardadas en el directorio: {charts_dir}")
    print("\nGráficas generadas:")
    print("- openmp_advanced_analysis.png: Cláusulas OpenMP avanzadas")
    print("- data_structures_analysis.png: Optimización de estructuras de datos")
    print("- memory_access_analysis.png: Optimización de acceso a memoria")
    print("- other_mechanisms_analysis.png: Otros mecanismos de optimización")
    print("- speedup_comparison.png: Comparación Base vs Optimizado")

if __name__ == "__main__":
    main()
