# Multiplicación de Matrices Optimizada - Reto 2

## Descripción del Proyecto

Este proyecto implementa múltiples versiones optimizadas de multiplicación de matrices con diferentes técnicas de paralelización y optimización de CPU y memoria. El objetivo es demostrar las mejoras de rendimiento obtenidas mediante optimizaciones a nivel de hardware y software.

## Implementaciones Incluidas

### 1. Versión Secuencial Optimizada (`multiplicacion_matrices.c`)
- **Optimizaciones implementadas:**
  - Cache blocking/tiling con bloques de 64x64
  - Uso de `memset` para inicialización eficiente
  - Reordenamiento de bucles para mejor localidad de cache
  - Comparación entre algoritmo original y optimizado

### 2. Versión OpenMP (`multiplicacion_openmp.c`)
- **Características:**
  - Paralelización automática con OpenMP
  - Generación paralela de matrices con semillas independientes
  - Cache blocking optimizado para paralelización
  - Múltiples estrategias de scheduling (static, dynamic)
  - Medición de eficiencia y speedup

### 3. Versión Pthread (`multiplicación_hilos.c`)
- **Optimizaciones:**
  - Uso de barreras para sincronización
  - Cache blocking por hilo (bloques de 32x32)
  - Generación paralela de matrices
  - Mejor distribución de trabajo entre hilos
  - Comparación entre versiones original y optimizada

### 4. Versión Procesos (`multiplicacion_procesos.c`)
- **Características:**
  - Memoria compartida con `mmap`
  - Cache blocking optimizado para procesos
  - Distribución eficiente de filas entre procesos
  - Aislamiento de memoria entre procesos

## Optimizaciones Implementadas

### Optimizaciones de CPU
1. **Cache Blocking/Tiling**: Reorganización de bucles para mejorar la localidad de datos
2. **Vectorización**: Hints para el compilador con flags de optimización
3. **Reordenamiento de bucles**: Mejor acceso a memoria secuencial
4. **Uso de registros**: Optimización de variables temporales

### Optimizaciones de Memoria
1. **Memoria compartida**: Uso de `mmap` para procesos
2. **Inicialización eficiente**: `memset` en lugar de bucles
3. **Localidad de cache**: Acceso secuencial a datos
4. **Reducción de fragmentación**: Asignación contigua de memoria

### Optimizaciones de Paralelización
1. **Load balancing**: Distribución equitativa de trabajo
2. **Sincronización eficiente**: Barreras y mutex optimizados
3. **Reducción de contención**: Minimización de acceso a memoria compartida
4. **Escalabilidad**: Soporte para diferentes números de hilos/procesos

## Compilación y Uso

### Requisitos del Sistema
```bash
# Ubuntu/Debian
sudo apt-get install build-essential gcc g++ make libomp-dev

# Verificar instalación
make check-system
```

### Compilación
```bash
# Compilar todas las versiones con optimización O3
make all

# Compilar con diferentes niveles de optimización
make optimize-o0    # Sin optimización
make optimize-o1    # Optimización básica
make optimize-o2    # Optimización estándar
make optimize-fast  # Optimizaciones agresivas

# Compilar para debugging
make debug

# Compilar para profiling
make profile
```

### Ejecución
```bash
# Versión secuencial
./build/matrices_seq 1000

# Versión OpenMP (4 hilos)
./build/matrices_openmp 1000 4

# Versión Pthread (4 hilos)
./build/matrices_pthread 1000 4

# Versión Procesos (4 procesos)
./build/matrices_procesos 1000 4
```

## Benchmarking y Profiling

### Ejecutar Benchmarks
```bash
# Benchmark completo (puede tomar mucho tiempo)
make benchmark

# Benchmark rápido
make benchmark-quick

# Benchmark de escalabilidad
make benchmark-scalability

# Ejecutar scripts directamente
./benchmark.sh full
./benchmark.sh quick
./scalability_test.sh
```

### Análisis de Rendimiento
Los benchmarks generan archivos CSV con métricas detalladas:
- **Tiempo de ejecución**: En segundos
- **Speedup**: Mejora relativa vs versión secuencial
- **Eficiencia**: Porcentaje de utilización de recursos
- **Uso de memoria**: Consumo de RAM durante ejecución

## Resultados Esperados

### Mejoras de Rendimiento
- **Cache blocking**: 2-5x speedup en matrices grandes
- **Paralelización**: Speedup lineal hasta el número de núcleos
- **OpenMP vs Pthread**: OpenMP generalmente más eficiente
- **Procesos vs Hilos**: Hilos más eficientes para comunicación

### Escalabilidad
- **Escalabilidad lineal**: Hasta número de núcleos físicos
- **Eficiencia**: 70-90% con carga balanceada
- **Overhead**: Mínimo con optimizaciones implementadas

## Análisis de Caracterización

### Métricas de CPU
- **Utilización**: Monitoreo de uso de núcleos
- **Cache misses**: Reducción mediante blocking
- **IPC**: Instrucciones por ciclo mejoradas

### Métricas de Memoria
- **Bandwidth**: Uso eficiente del ancho de banda
- **Latency**: Reducción de latencia de acceso
- **Fragmentation**: Minimización de fragmentación

## Estructura del Proyecto

```
reto2/
├── multiplicacion_matrices.c      # Versión secuencial optimizada
├── multiplicacion_openmp.c        # Versión OpenMP
├── multiplicación_hilos.c         # Versión Pthread
├── multiplicacion_procesos.c      # Versión procesos
├── Makefile                       # Sistema de compilación
├── benchmark.sh                   # Script de benchmarking
├── scalability_test.sh           # Pruebas de escalabilidad
├── README.md                      # Esta documentación
├── build/                         # Ejecutables compilados
└── results/                       # Resultados de benchmarks
```

## Comandos Útiles

```bash
# Ver ayuda completa
make help

# Limpiar archivos compilados
make clean

# Instalar dependencias
make install-deps

# Verificar sistema
make check-system

# Compilar con profiling
make profile
gprof build/matrices_seq gmon.out > profile.txt
```

## Análisis de Resultados

### Interpretación de Métricas
- **Speedup > 1**: Mejora de rendimiento
- **Eficiencia > 70%**: Buena utilización de recursos
- **Escalabilidad lineal**: Rendimiento proporcional a recursos

### Comparación entre Implementaciones
1. **Secuencial**: Baseline para comparaciones
2. **OpenMP**: Fácil de usar, buen rendimiento
3. **Pthread**: Control fino, overhead menor
4. **Procesos**: Aislamiento, mayor overhead

## Troubleshooting

### Problemas Comunes
```bash
# Error de compilación OpenMP
export OMP_NUM_THREADS=4

# Error de memoria
ulimit -s unlimited

# Error de permisos
chmod +x benchmark.sh scalability_test.sh
```

### Optimización Adicional
- Ajustar `BLOCK_SIZE` según arquitectura
- Modificar flags de compilación
- Experimentar con diferentes schedulers
- Usar herramientas de profiling (perf, valgrind)

## Conclusiones

Este proyecto demuestra la importancia de las optimizaciones a nivel de CPU y memoria en aplicaciones de cómputo intensivo. Las técnicas implementadas muestran mejoras significativas en rendimiento, especialmente en matrices grandes donde la localidad de cache es crítica.

Las implementaciones paralelas (OpenMP, Pthread, Procesos) permiten aprovechar múltiples núcleos de CPU, obteniendo speedups lineales hasta el número de núcleos disponibles, con eficiencias típicas del 70-90%.
