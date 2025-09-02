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

Cada gráfica se enfoca en demostrar la mejora del speedup de la versión optimizada
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
rcParams['font.size'] = 12
rcParams['axes.titlesize'] = 16
rcParams['axes.labelsize'] = 14
rcParams['xtick.labelsize'] = 12
rcParams['ytick.labelsize'] = 12
rcParams['legend.fontsize'] = 12

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
    """
    Inciso A: Uso de cláusulas y directivas de OpenMP no vistas en clase
    Gráfica que muestra el speedup por número de hilos comparando Base vs Optimizado
    """
    plt.figure(figsize=(12, 8))
    
    # Filtrar solo implementaciones paralelas
    parallel_df = df[df['Implementation'] != 'SECUENCIAL']
    
    # Agrupar datos por número de hilos e implementación
    speedup_by_threads = parallel_df.groupby(['NumThreads', 'Implementation'])['Speedup'].mean().unstack()
    
    # Crear la gráfica
    ax = speedup_by_threads.plot(kind='line', marker='o', linewidth=4, markersize=10, figsize=(12, 8))
    
    # Configurar la gráfica
    plt.title('Inciso A: Cláusulas OpenMP Avanzadas\nSpeedup vs Número de Hilos', fontsize=18, pad=20)
    plt.xlabel('Número de Hilos', fontsize=14)
    plt.ylabel('Speedup', fontsize=14)
    plt.grid(True, alpha=0.3, linestyle='--')
    
    # Personalizar colores
    colors = ['#FF6B6B', '#4ECDC4']
    for i, line in enumerate(ax.get_lines()):
        line.set_color(colors[i])
        line.set_linewidth(4)
        line.set_markersize(10)
    
    plt.legend(title='Implementación', title_fontsize=12, fontsize=12, 
               loc='upper left', framealpha=0.9, shadow=True)
    
    for impl in speedup_by_threads.columns:
        for threads, speedup in speedup_by_threads[impl].items():
            if not pd.isna(speedup):
                color = colors[0] if 'BASE' in impl else colors[1]
                plt.annotate(f'{speedup:.2f}x', 
                           (threads, speedup), 
                           textcoords="offset points", 
                           xytext=(0,10), 
                           ha='center', va='bottom',
                           fontweight='bold',
                           color=color)
    
    base_avg = speedup_by_threads.get('PARALELO_BASE', pd.Series()).mean()
    opt_avg = speedup_by_threads.get('PARALELO_OPTIMIZADO', pd.Series()).mean()
    
    if not pd.isna(base_avg) and not pd.isna(opt_avg):
        improvement = ((opt_avg - base_avg) / base_avg) * 100
        plt.text(0.02, 0.98, f'Mejora Promedio: {improvement:.1f}%', 
                transform=ax.transAxes, fontsize=12, fontweight='bold',
                verticalalignment='top', bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))
    
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight', facecolor='white')
    print(f"Gráfica OpenMP avanzado guardada: {output_file}")
    plt.close()

def create_data_structures_chart(df, output_file="data_structures_analysis.png"):
    """
    Inciso B: Optimización en la creación y uso de estructuras de datos
    Gráfica de barras mostrando speedup promedio por implementación
    """
    plt.figure(figsize=(12, 8))
    
    # Filtrar solo implementaciones paralelas
    parallel_df = df[df['Implementation'] != 'SECUENCIAL']
    
    # Calcular speedup promedio por implementación
    speedup_avg = parallel_df.groupby('Implementation')['Speedup'].mean().sort_values(ascending=False)
    
    # Crear la gráfica de barras
    colors = ['#4ECDC4' if 'OPTIMIZADO' in impl else '#FF6B6B' for impl in speedup_avg.index]
    bars = plt.bar(speedup_avg.index, speedup_avg.values, color=colors, alpha=0.8, width=0.6)
    
    # Configurar la gráfica
    plt.title('Inciso B: Optimización de Estructuras de Datos\nSpeedup Promedio por Implementación', 
              fontsize=18, pad=20)
    plt.ylabel('Speedup Promedio', fontsize=14)
    plt.xlabel('Implementación', fontsize=14)
    plt.grid(True, alpha=0.3, axis='y', linestyle='--')
    
    for bar, value in zip(bars, speedup_avg.values):
        height = bar.get_height()
        plt.text(bar.get_x() + bar.get_width()/2., height + 0.05,
                f'{value:.2f}x', ha='center', va='bottom', 
                fontsize=12, fontweight='bold')
    
    if len(speedup_avg) >= 2:
        base_speedup = speedup_avg.get('PARALELO_BASE', 0)
        opt_speedup = speedup_avg.get('PARALELO_OPTIMIZADO', 0)
        
        if base_speedup > 0:
            improvement = ((opt_speedup - base_speedup) / base_speedup) * 100
            plt.text(0.02, 0.98, f'Mejora: {improvement:.1f}%\n({opt_speedup:.2f}x vs {base_speedup:.2f}x)', 
                    transform=plt.gca().transAxes, fontsize=12, fontweight='bold',
                    verticalalignment='top', bbox=dict(boxstyle='round', facecolor='lightgreen', alpha=0.8))
    
    plt.xticks(rotation=0)
    
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight', facecolor='white')
    print(f"Gráfica estructuras de datos guardada: {output_file}")
    plt.close()

def create_memory_access_chart(df, output_file="memory_access_analysis.png"):
    """
    Inciso C: Optimización en los mecanismos de acceso a memoria
    Gráfica que muestra el speedup por tamaño de problema (número de círculos)
    """
    plt.figure(figsize=(12, 8))
    
    # Filtrar solo implementaciones paralelas
    parallel_df = df[df['Implementation'] != 'SECUENCIAL']
    
    # Agrupar datos por número de círculos e implementación
    speedup_by_circles = parallel_df.groupby(['NumCircles', 'Implementation'])['Speedup'].mean().unstack()
    
    # Crear gráfica de barras agrupadas
    ax = speedup_by_circles.plot(kind='bar', figsize=(12, 8), width=0.8, alpha=0.8)
    
    # Configurar colores
    colors = ['#FF6B6B', '#4ECDC4']
    for i, bar_group in enumerate(ax.containers):
        for bar in bar_group:
            bar.set_color(colors[i])
    
    # Configurar la gráfica
    plt.title('Inciso C: Optimización de Acceso a Memoria\nSpeedup vs Tamaño del Problema', 
              fontsize=18, pad=20)
    plt.xlabel('Número de Círculos', fontsize=14)
    plt.ylabel('Speedup', fontsize=14)
    plt.grid(True, alpha=0.3, axis='y', linestyle='--')
    
    plt.legend(title='Implementación', title_fontsize=12, fontsize=12, 
               loc='upper left', framealpha=0.9, shadow=True)
    
    for container in ax.containers:
        ax.bar_label(container, fmt='%.2fx', padding=3, fontweight='bold')
    
    plt.xticks(rotation=45)
    
    base_avg = speedup_by_circles.get('PARALELO_BASE', pd.Series()).mean()
    opt_avg = speedup_by_circles.get('PARALELO_OPTIMIZADO', pd.Series()).mean()
    
    if not pd.isna(base_avg) and not pd.isna(opt_avg):
        improvement = ((opt_avg - base_avg) / base_avg) * 100
        plt.text(0.02, 0.98, f'Mejora Promedio: {improvement:.1f}%', 
                transform=ax.transAxes, fontsize=12, fontweight='bold',
                verticalalignment='top', bbox=dict(boxstyle='round', facecolor='lightblue', alpha=0.8))
    
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight', facecolor='white')
    print(f"Gráfica acceso a memoria guardada: {output_file}")
    plt.close()

def create_other_mechanisms_chart(df, output_file="other_mechanisms_analysis.png"):
    """
    Inciso D: Cualquier otro mecanismo de optimización
    Gráfica de líneas mostrando eficiencia por número de hilos
    """
    plt.figure(figsize=(12, 8))
    
    # Filtrar solo implementaciones paralelas
    parallel_df = df[df['Implementation'] != 'SECUENCIAL']
    
    # Agrupar datos por número de hilos e implementación para eficiencia
    efficiency_by_threads = parallel_df.groupby(['NumThreads', 'Implementation'])['Efficiency'].mean().unstack()
    
    ax = efficiency_by_threads.plot(kind='line', marker='s', linewidth=4, markersize=10, figsize=(12, 8))
    
    # Configurar la gráfica
    plt.title('Inciso D: Otros Mecanismos de Optimización\nEficiencia vs Número de Hilos', 
              fontsize=18, pad=20)
    plt.xlabel('Número de Hilos', fontsize=14)
    plt.ylabel('Eficiencia (%)', fontsize=14)
    plt.grid(True, alpha=0.3, linestyle='--')
    
    # Personalizar colores
    colors = ['#FF6B6B', '#4ECDC4']
    for i, line in enumerate(ax.get_lines()):
        line.set_color(colors[i])
        line.set_linewidth(4)
        line.set_markersize(10)

    plt.legend(title='Implementación', title_fontsize=12, fontsize=12, 
               loc='upper right', framealpha=0.9, shadow=True)
    
    for impl in efficiency_by_threads.columns:
        for threads, efficiency in efficiency_by_threads[impl].items():
            if not pd.isna(efficiency):
                color = colors[0] if 'BASE' in impl else colors[1]
                plt.annotate(f'{efficiency:.1f}%', 
                           (threads, efficiency), 
                           textcoords="offset points", 
                           xytext=(0,10), 
                           ha='center', va='bottom',
                           fontweight='bold',
                           color=color)
    
    base_avg = efficiency_by_threads.get('PARALELO_BASE', pd.Series()).mean()
    opt_avg = efficiency_by_threads.get('PARALELO_OPTIMIZADO', pd.Series()).mean()
    
    if not pd.isna(base_avg) and not pd.isna(opt_avg):
        improvement = opt_avg - base_avg
        plt.text(0.02, 0.02, f'Mejora en Eficiencia: +{improvement:.1f} puntos porcentuales', 
                transform=ax.transAxes, fontsize=12, fontweight='bold',
                verticalalignment='bottom', bbox=dict(boxstyle='round', facecolor='lightyellow', alpha=0.8))
    
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight', facecolor='white')
    print(f"Gráfica otros mecanismos guardada: {output_file}")
    plt.close()

def create_speedup_comparison_chart(df, output_file="speedup_comparison.png"):
    """
    Comparación general: Speedup máximo alcanzado por cada implementación
    """
    plt.figure(figsize=(12, 8))
    
    # Filtrar solo implementaciones paralelas
    parallel_df = df[df['Implementation'] != 'SECUENCIAL']
    
    # Calcular speedup máximo por implementación
    max_speedup = parallel_df.groupby('Implementation')['Speedup'].max().sort_values(ascending=True)
    
    # Crear gráfica horizontal
    colors = ['#FF6B6B' if 'BASE' in impl else '#4ECDC4' for impl in max_speedup.index]
    bars = plt.barh(max_speedup.index, max_speedup.values, color=colors, alpha=0.8, height=0.6)
    
    # Configurar la gráfica
    plt.title('Comparación General: Speedup Máximo Alcanzado\nBase vs Optimizado', 
              fontsize=18, pad=20)
    plt.xlabel('Speedup Máximo', fontsize=14)
    plt.ylabel('Implementación', fontsize=14)
    plt.grid(True, alpha=0.3, axis='x', linestyle='--')
    
    for bar, value in zip(bars, max_speedup.values):
        width = bar.get_width()
        plt.text(width + 0.05, bar.get_y() + bar.get_height()/2.,
                f'{value:.2f}x', ha='left', va='center', 
                fontsize=12, fontweight='bold')
    
    if len(max_speedup) >= 2:
        base_max = max_speedup.get('PARALELO_BASE', 0)
        opt_max = max_speedup.get('PARALELO_OPTIMIZADO', 0)
        
        if base_max > 0:
            improvement = ((opt_max - base_max) / base_max) * 100
            plt.text(0.02, 0.98, f'Mejora en Speedup Máximo: {improvement:.1f}%\n({opt_max:.2f}x vs {base_max:.2f}x)', 
                    transform=plt.gca().transAxes, fontsize=12, fontweight='bold',
                    verticalalignment='top', bbox=dict(boxstyle='round', facecolor='lightcoral', alpha=0.8))
    
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight', facecolor='white')
    print(f"Gráfica de comparación de speedup guardada: {output_file}")
    plt.close()

def main():
    """Función principal"""
    csv_file = "src/data/main_optimized.csv"
    
    if len(sys.argv) > 1:
        csv_file = sys.argv[1]
    
    print("GENERADOR DE GRAFICAS PARA ANALISIS DE OPTIMIZACIONES PARALELAS")
    print("=============================================================")
    print(f"Archivo CSV: {csv_file}")
    print()
    
    df = load_data(csv_file)
    if df is None:
        return
    
    charts_dir = "src/charts"
    if not os.path.exists(charts_dir):
        os.makedirs(charts_dir)
    
    print("\nGenerando gráficas enfocadas en speedup...")
    
    create_openmp_advanced_chart(df, f"{charts_dir}/openmp_advanced_analysis.png")
    create_data_structures_chart(df, f"{charts_dir}/data_structures_analysis.png")
    create_memory_access_chart(df, f"{charts_dir}/memory_access_analysis.png")
    create_other_mechanisms_chart(df, f"{charts_dir}/other_mechanisms_analysis.png")
    create_speedup_comparison_chart(df, f"{charts_dir}/speedup_comparison.png")
    
    print(f"\nTodas las gráficas han sido guardadas en el directorio: {charts_dir}")
    print("\nGráficas generadas (enfocadas en demostrar mejoras):")
    print("- openmp_advanced_analysis.png: Inciso A - Cláusulas OpenMP (Speedup vs Hilos)")
    print("- data_structures_analysis.png: Inciso B - Estructuras de Datos (Speedup Promedio)")
    print("- memory_access_analysis.png: Inciso C - Acceso a Memoria (Speedup vs Tamaño)")
    print("- other_mechanisms_analysis.png: Inciso D - Otros Mecanismos (Eficiencia vs Hilos)")
    print("- speedup_comparison.png: Comparación General (Speedup Máximo)")
    
    import glob
    png_files = glob.glob(f"{charts_dir}/*.png")
    print(f"\nVerificación: {len(png_files)} gráficas encontradas:")
    for png_file in png_files:
        print(f"  - {png_file}")
    
    print(f"\nResumen de datos para validación:")
    parallel_df = df[df['Implementation'] != 'SECUENCIAL']
    if not parallel_df.empty:
        summary = parallel_df.groupby('Implementation').agg({
            'Speedup': ['mean', 'max', 'min'],
            'Efficiency': 'mean',
            'ExecutionTime': 'mean'
        }).round(3)
        print(summary)

if __name__ == "__main__":
    main()