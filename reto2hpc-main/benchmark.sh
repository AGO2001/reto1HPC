#!/bin/bash

# Script de Benchmarking para Multiplicación de Matrices Optimizada
# Ejecuta pruebas de rendimiento en diferentes configuraciones

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuración
BUILD_DIR="build"
RESULTS_DIR="results"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
RESULTS_FILE="$RESULTS_DIR/benchmark_$TIMESTAMP.txt"

# Tamaños de matriz a probar
MATRIX_SIZES=(500 1000 1500 2000)
THREAD_COUNTS=(1 2 4 8)

# Función para mostrar información del sistema
show_system_info() {
    echo -e "${BLUE}=== INFORMACIÓN DEL SISTEMA ===${NC}" | tee -a "$RESULTS_FILE"
    echo "Fecha: $(date)" | tee -a "$RESULTS_FILE"
    echo "Hostname: $(hostname)" | tee -a "$RESULTS_FILE"
    echo "Procesador: $(lscpu | grep 'Model name' | cut -d: -f2 | xargs)" | tee -a "$RESULTS_FILE"
    echo "Núcleos físicos: $(lscpu | grep 'Socket(s)' | awk '{print $2}')" | tee -a "$RESULTS_FILE"
    echo "Núcleos lógicos: $(nproc)" | tee -a "$RESULTS_FILE"
    echo "Memoria total: $(free -h | grep 'Mem:' | awk '{print $2}')" | tee -a "$RESULTS_FILE"
    echo "Memoria disponible: $(free -h | grep 'Mem:' | awk '{print $7}')" | tee -a "$RESULTS_FILE"
    echo "Versión GCC: $(gcc --version | head -n1)" | tee -a "$RESULTS_FILE"
    echo "" | tee -a "$RESULTS_FILE"
}

# Función para verificar que los ejecutables existen
check_executables() {
    local missing=0
    
    if [ ! -f "$BUILD_DIR/matrices_seq" ]; then
        echo -e "${RED}Error: $BUILD_DIR/matrices_seq no encontrado${NC}"
        missing=1
    fi
    
    if [ ! -f "$BUILD_DIR/matrices_openmp" ]; then
        echo -e "${RED}Error: $BUILD_DIR/matrices_openmp no encontrado${NC}"
        missing=1
    fi
    
    if [ ! -f "$BUILD_DIR/matrices_pthread" ]; then
        echo -e "${RED}Error: $BUILD_DIR/matrices_pthread no encontrado${NC}"
        missing=1
    fi
    
    if [ ! -f "$BUILD_DIR/matrices_procesos" ]; then
        echo -e "${RED}Error: $BUILD_DIR/matrices_procesos no encontrado${NC}"
        missing=1
    fi
    
    if [ $missing -eq 1 ]; then
        echo -e "${YELLOW}Ejecuta 'make all' para compilar los ejecutables${NC}"
        exit 1
    fi
}

# Función para ejecutar una prueba y capturar resultados
run_test() {
    local executable="$1"
    local matrix_size="$2"
    local thread_count="$3"
    local test_name="$4"
    
    echo -e "${YELLOW}Ejecutando: $test_name (${matrix_size}x${matrix_size}, $thread_count hilos/procesos)${NC}"
    
    # Crear archivo temporal para capturar output
    local temp_file=$(mktemp)
    
    # Ejecutar con timeout de 5 minutos
    timeout 300s "$executable" "$matrix_size" "$thread_count" > "$temp_file" 2>&1
    local exit_code=$?
    
    if [ $exit_code -eq 124 ]; then
        echo -e "${RED}TIMEOUT: $test_name${NC}" | tee -a "$RESULTS_FILE"
        echo "Tiempo: > 300 segundos" | tee -a "$RESULTS_FILE"
    elif [ $exit_code -eq 0 ]; then
        # Extraer tiempo de ejecución
        local time_result=$(grep "Tiempo de multiplicación" "$temp_file" | tail -1 | awk '{print $4}')
        local memory_result=$(grep "Memoria final" "$temp_file" | tail -1 | awk '{print $3, $4}')
        
        echo -e "${GREEN}✓ $test_name completado${NC}"
        echo "Tiempo: $time_result segundos" | tee -a "$RESULTS_FILE"
        echo "Memoria: $memory_result" | tee -a "$RESULTS_FILE"
        
        # Extraer speedup si está disponible
        local speedup=$(grep "Speedup:" "$temp_file" | tail -1 | awk '{print $2}')
        if [ ! -z "$speedup" ]; then
            echo "Speedup: ${speedup}x" | tee -a "$RESULTS_FILE"
        fi
    else
        echo -e "${RED}ERROR: $test_name (código: $exit_code)${NC}" | tee -a "$RESULTS_FILE"
        echo "Error details:" | tee -a "$RESULTS_FILE"
        cat "$temp_file" | tee -a "$RESULTS_FILE"
    fi
    
    echo "" | tee -a "$RESULTS_FILE"
    rm -f "$temp_file"
}

# Función principal de benchmarking
run_benchmark() {
    echo -e "${BLUE}=== INICIANDO BENCHMARK COMPLETO ===${NC}"
    echo "Resultados se guardarán en: $RESULTS_FILE"
    echo ""
    
    # Crear directorio de resultados si no existe
    mkdir -p "$RESULTS_DIR"
    
    # Mostrar información del sistema
    show_system_info
    
    # Verificar ejecutables
    check_executables
    
    # Benchmark secuencial
    echo -e "${BLUE}=== BENCHMARK SECUENCIAL ===${NC}" | tee -a "$RESULTS_FILE"
    for size in "${MATRIX_SIZES[@]}"; do
        run_test "$BUILD_DIR/matrices_seq" "$size" "1" "Secuencial"
    done
    
    # Benchmark OpenMP
    echo -e "${BLUE}=== BENCHMARK OPENMP ===${NC}" | tee -a "$RESULTS_FILE"
    for size in "${MATRIX_SIZES[@]}"; do
        for threads in "${THREAD_COUNTS[@]}"; do
            if [ "$threads" -le "$size" ]; then
                run_test "$BUILD_DIR/matrices_openmp" "$size" "$threads" "OpenMP"
            fi
        done
    done
    
    # Benchmark Pthread
    echo -e "${BLUE}=== BENCHMARK PTHREAD ===${NC}" | tee -a "$RESULTS_FILE"
    for size in "${MATRIX_SIZES[@]}"; do
        for threads in "${THREAD_COUNTS[@]}"; do
            if [ "$threads" -le "$size" ]; then
                run_test "$BUILD_DIR/matrices_pthread" "$size" "$threads" "Pthread"
            fi
        done
    done
    
    # Benchmark Procesos
    echo -e "${BLUE}=== BENCHMARK PROCESOS ===${NC}" | tee -a "$RESULTS_FILE"
    for size in "${MATRIX_SIZES[@]}"; do
        for threads in "${THREAD_COUNTS[@]}"; do
            if [ "$threads" -le "$size" ]; then
                run_test "$BUILD_DIR/matrices_procesos" "$size" "$threads" "Procesos"
            fi
        done
    done
    
    echo -e "${GREEN}=== BENCHMARK COMPLETADO ===${NC}"
    echo "Resultados guardados en: $RESULTS_FILE"
}

# Función para benchmark rápido
run_quick_benchmark() {
    echo -e "${BLUE}=== BENCHMARK RÁPIDO ===${NC}"
    local quick_file="$RESULTS_DIR/benchmark_quick_$TIMESTAMP.txt"
    
    mkdir -p "$RESULTS_DIR"
    show_system_info > "$quick_file"
    
    echo -e "${BLUE}=== BENCHMARK RÁPIDO (500x500) ===${NC}" | tee -a "$quick_file"
    
    # Solo probar con matriz 500x500 y 4 hilos/procesos
    run_test "$BUILD_DIR/matrices_seq" "500" "1" "Secuencial" "$quick_file"
    run_test "$BUILD_DIR/matrices_openmp" "500" "4" "OpenMP" "$quick_file"
    run_test "$BUILD_DIR/matrices_pthread" "500" "4" "Pthread" "$quick_file"
    run_test "$BUILD_DIR/matrices_procesos" "500" "4" "Procesos" "$quick_file"
    
    echo -e "${GREEN}Benchmark rápido completado${NC}"
    echo "Resultados en: $quick_file"
}

# Función para benchmark de escalabilidad
run_scalability_benchmark() {
    echo -e "${BLUE}=== BENCHMARK DE ESCALABILIDAD ===${NC}"
    local scalability_file="$RESULTS_DIR/scalability_$TIMESTAMP.txt"
    
    mkdir -p "$RESULTS_DIR"
    show_system_info > "$scalability_file"
    
    local max_threads=$(nproc)
    local test_size=1000
    
    echo -e "${BLUE}=== ESCALABILIDAD (${test_size}x${test_size}) ===${NC}" | tee -a "$scalability_file"
    
    # Probar con diferentes números de hilos
    for threads in 1 2 4 8 16; do
        if [ "$threads" -le "$max_threads" ] && [ "$threads" -le "$test_size" ]; then
            echo -e "${YELLOW}Probando con $threads hilos/procesos${NC}"
            
            echo "--- OpenMP con $threads hilos ---" | tee -a "$scalability_file"
            run_test "$BUILD_DIR/matrices_openmp" "$test_size" "$threads" "OpenMP" "$scalability_file"
            
            echo "--- Pthread con $threads hilos ---" | tee -a "$scalability_file"
            run_test "$BUILD_DIR/matrices_pthread" "$test_size" "$threads" "Pthread" "$scalability_file"
            
            echo "--- Procesos con $threads procesos ---" | tee -a "$scalability_file"
            run_test "$BUILD_DIR/matrices_procesos" "$test_size" "$threads" "Procesos" "$scalability_file"
        fi
    done
    
    echo -e "${GREEN}Benchmark de escalabilidad completado${NC}"
    echo "Resultados en: $scalability_file"
}

# Función para mostrar ayuda
show_help() {
    echo "Uso: $0 [OPCIÓN]"
    echo ""
    echo "Opciones:"
    echo "  full        - Ejecutar benchmark completo (por defecto)"
    echo "  quick       - Ejecutar benchmark rápido"
    echo "  scalability - Ejecutar benchmark de escalabilidad"
    echo "  help        - Mostrar esta ayuda"
    echo ""
    echo "Ejemplos:"
    echo "  $0 full"
    echo "  $0 quick"
    echo "  $0 scalability"
}

# Función para limpiar archivos temporales
cleanup() {
    rm -f /tmp/benchmark_*
}

# Capturar señales para limpiar
trap cleanup EXIT

# Procesar argumentos
case "${1:-full}" in
    "full")
        run_benchmark
        ;;
    "quick")
        run_quick_benchmark
        ;;
    "scalability")
        run_scalability_benchmark
        ;;
    "help"|"-h"|"--help")
        show_help
        ;;
    *)
        echo -e "${RED}Opción desconocida: $1${NC}"
        show_help
        exit 1
        ;;
esac
