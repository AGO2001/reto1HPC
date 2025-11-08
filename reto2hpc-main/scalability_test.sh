#!/bin/bash

# Script de Prueba de Escalabilidad para Multiplicación de Matrices
# Analiza el rendimiento con diferentes números de hilos/procesos

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
RESULTS_FILE="$RESULTS_DIR/scalability_$TIMESTAMP.csv"

# Configuración de pruebas
MATRIX_SIZE=1000
MAX_THREADS=$(nproc)
THREAD_SEQUENCE=(1 2 4 8 16 32)

# Función para extraer tiempo de ejecución
extract_time() {
    local output="$1"
    echo "$output" | grep "Tiempo de multiplicación" | tail -1 | awk '{print $4}'
}

# Función para extraer speedup
extract_speedup() {
    local output="$1"
    echo "$output" | grep "Speedup:" | tail -1 | awk '{print $2}' | sed 's/x//'
}

# Función para extraer eficiencia
extract_efficiency() {
    local output="$1"
    echo "$output" | grep "Eficiencia:" | tail -1 | awk '{print $2}' | sed 's/%//'
}

# Función para ejecutar prueba de escalabilidad
run_scalability_test() {
    local executable="$1"
    local name="$2"
    local threads="$3"
    
    echo -e "${YELLOW}Probando $name con $threads hilos/procesos...${NC}"
    
    # Ejecutar y capturar output
    local output
    if [ "$name" = "Secuencial" ]; then
        output=$(timeout 300s "$executable" "$MATRIX_SIZE" 2>&1)
    else
        output=$(timeout 300s "$executable" "$MATRIX_SIZE" "$threads" 2>&1)
    fi
    
    local exit_code=$?
    
    if [ $exit_code -eq 124 ]; then
        echo -e "${RED}TIMEOUT: $name con $threads hilos${NC}"
        echo "$name,$threads,TIMEOUT,TIMEOUT,TIMEOUT" >> "$RESULTS_FILE"
    elif [ $exit_code -eq 0 ]; then
        local time_result=$(extract_time "$output")
        local speedup_result=$(extract_speedup "$output")
        local efficiency_result=$(extract_efficiency "$output")
        
        echo -e "${GREEN}✓ $name con $threads hilos: ${time_result}s${NC}"
        echo "$name,$threads,$time_result,$speedup_result,$efficiency_result" >> "$RESULTS_FILE"
    else
        echo -e "${RED}ERROR: $name con $threads hilos (código: $exit_code)${NC}"
        echo "$name,$threads,ERROR,ERROR,ERROR" >> "$RESULTS_FILE"
    fi
}

# Función principal
main() {
    echo -e "${BLUE}=== PRUEBA DE ESCALABILIDAD ===${NC}"
    echo "Tamaño de matriz: ${MATRIX_SIZE}x${MATRIX_SIZE}"
    echo "Máximo de hilos disponibles: $MAX_THREADS"
    echo "Resultados se guardarán en: $RESULTS_FILE"
    echo ""
    
    # Crear directorio de resultados
    mkdir -p "$RESULTS_DIR"
    
    # Verificar ejecutables
    if [ ! -f "$BUILD_DIR/matrices_seq" ] || [ ! -f "$BUILD_DIR/matrices_openmp" ] || 
       [ ! -f "$BUILD_DIR/matrices_pthread" ] || [ ! -f "$BUILD_DIR/matrices_procesos" ]; then
        echo -e "${RED}Error: Ejecutables no encontrados. Ejecuta 'make all' primero.${NC}"
        exit 1
    fi
    
    # Crear archivo CSV con headers
    echo "Implementación,Hilos,Tiempo(s),Speedup,Eficiencia(%)" > "$RESULTS_FILE"
    
    # Prueba secuencial (baseline)
    echo -e "${BLUE}=== LÍNEA BASE (SECUENCIAL) ===${NC}"
    run_scalability_test "$BUILD_DIR/matrices_seq" "Secuencial" "1"
    
    # Pruebas paralelas
    echo -e "${BLUE}=== PRUEBAS PARALELAS ===${NC}"
    for threads in "${THREAD_SEQUENCE[@]}"; do
        if [ "$threads" -le "$MAX_THREADS" ] && [ "$threads" -le "$MATRIX_SIZE" ]; then
            echo -e "${YELLOW}--- Probando con $threads hilos/procesos ---${NC}"
            
            # OpenMP
            run_scalability_test "$BUILD_DIR/matrices_openmp" "OpenMP" "$threads"
            
            # Pthread
            run_scalability_test "$BUILD_DIR/matrices_pthread" "Pthread" "$threads"
            
            # Procesos
            run_scalability_test "$BUILD_DIR/matrices_procesos" "Procesos" "$threads"
            
            echo ""
        fi
    done
    
    # Generar resumen
    echo -e "${GREEN}=== RESUMEN DE ESCALABILIDAD ===${NC}"
    echo "Resultados guardados en: $RESULTS_FILE"
    echo ""
    echo "Mejores resultados por implementación:"
    
    # Mostrar mejores resultados
    echo -e "${BLUE}OpenMP:${NC}"
    grep "OpenMP" "$RESULTS_FILE" | grep -v "ERROR\|TIMEOUT" | sort -t',' -k3 -n | head -3
    
    echo -e "${BLUE}Pthread:${NC}"
    grep "Pthread" "$RESULTS_FILE" | grep -v "ERROR\|TIMEOUT" | sort -t',' -k3 -n | head -3
    
    echo -e "${BLUE}Procesos:${NC}"
    grep "Procesos" "$RESULTS_FILE" | grep -v "ERROR\|TIMEOUT" | sort -t',' -k3 -n | head -3
    
    echo ""
    echo -e "${GREEN}Prueba de escalabilidad completada${NC}"
}

# Ejecutar función principal
main "$@"
