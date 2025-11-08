# Makefile para Multiplicación de Matrices Optimizada
# Incluye diferentes niveles de optimización y profiling

# Compilador y flags base
CC = gcc
CXX = g++
CFLAGS_BASE = -Wall -Wextra -std=c99
CXXFLAGS_BASE = -Wall -Wextra -std=c++11

# Flags de optimización
CFLAGS_O0 = $(CFLAGS_BASE) -O0 -g
CFLAGS_O1 = $(CFLAGS_BASE) -O1
CFLAGS_O2 = $(CFLAGS_BASE) -O2
CFLAGS_O3 = $(CFLAGS_BASE) -O3 -march=native -mtune=native
CFLAGS_FAST = $(CFLAGS_BASE) -O3 -march=native -mtune=native -ffast-math -funroll-loops

# Flags para profiling
CFLAGS_PROFILE = $(CFLAGS_O2) -pg -fprofile-arcs -ftest-coverage
CFLAGS_DEBUG = $(CFLAGS_BASE) -O0 -g -DDEBUG

# Flags específicos para OpenMP
CFLAGS_OPENMP = -fopenmp

# Flags específicos para pthread
CFLAGS_PTHREAD = -pthread

# Librerías
LIBS = -lm
LIBS_OPENMP = -fopenmp
LIBS_PTHREAD = -pthread

# Directorio de salida
BUILD_DIR = build
RESULTS_DIR = results

# Archivos fuente
SOURCES = multiplicacion_matrices.c multiplicacion_openmp.c multiplicación_hilos.c multiplicacion_procesos.c
EXECUTABLES = $(BUILD_DIR)/matrices_seq $(BUILD_DIR)/matrices_openmp $(BUILD_DIR)/matrices_pthread $(BUILD_DIR)/matrices_procesos

# Reglas principales
.PHONY: all clean debug profile benchmark help install-deps

all: $(BUILD_DIR) $(RESULTS_DIR) $(EXECUTABLES)

# Crear directorios
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(RESULTS_DIR):
	mkdir -p $(RESULTS_DIR)

# Compilación con diferentes niveles de optimización
$(BUILD_DIR)/matrices_seq: multiplicacion_matrices.c | $(BUILD_DIR)
	$(CXX) $(CFLAGS_O3) -o $@ $< $(LIBS)

$(BUILD_DIR)/matrices_openmp: multiplicacion_openmp.c | $(BUILD_DIR)
	$(CXX) $(CFLAGS_O3) $(CFLAGS_OPENMP) -o $@ $< $(LIBS) $(LIBS_OPENMP)

$(BUILD_DIR)/matrices_pthread: multiplicación_hilos.c | $(BUILD_DIR)
	$(CXX) $(CFLAGS_O3) $(CFLAGS_PTHREAD) -o $@ $< $(LIBS) $(LIBS_PTHREAD)

$(BUILD_DIR)/matrices_procesos: multiplicacion_procesos.c | $(BUILD_DIR)
	$(CXX) $(CFLAGS_O3) -o $@ $< $(LIBS)

# Versiones de debug
debug: CFLAGS_O3 = $(CFLAGS_DEBUG)
debug: $(EXECUTABLES)

# Versiones para profiling
profile: CFLAGS_O3 = $(CFLAGS_PROFILE)
profile: $(EXECUTABLES)

# Versiones con diferentes niveles de optimización
optimize-o0: CFLAGS_O3 = $(CFLAGS_O0)
optimize-o0: $(EXECUTABLES)

optimize-o1: CFLAGS_O3 = $(CFLAGS_O1)
optimize-o1: $(EXECUTABLES)

optimize-o2: CFLAGS_O3 = $(CFLAGS_O2)
optimize-o2: $(EXECUTABLES)

optimize-fast: CFLAGS_O3 = $(CFLAGS_FAST)
optimize-fast: $(EXECUTABLES)

# Script de benchmarking
benchmark: $(EXECUTABLES)
	@echo "Ejecutando benchmark completo..."
	@chmod +x benchmark.sh
	./benchmark.sh

# Benchmark rápido (solo tamaños pequeños)
benchmark-quick: $(EXECUTABLES)
	@echo "Ejecutando benchmark rápido..."
	@echo "=== BENCHMARK RÁPIDO ===" > $(RESULTS_DIR)/benchmark_quick.txt
	@echo "Tamaño: 500x500, Hilos/Procesos: 4" >> $(RESULTS_DIR)/benchmark_quick.txt
	@echo "" >> $(RESULTS_DIR)/benchmark_quick.txt
	@echo "--- Secuencial ---" >> $(RESULTS_DIR)/benchmark_quick.txt
	@time -p $(BUILD_DIR)/matrices_seq 500 2>> $(RESULTS_DIR)/benchmark_quick.txt
	@echo "" >> $(RESULTS_DIR)/benchmark_quick.txt
	@echo "--- OpenMP ---" >> $(RESULTS_DIR)/benchmark_quick.txt
	@time -p $(BUILD_DIR)/matrices_openmp 500 4 2>> $(RESULTS_DIR)/benchmark_quick.txt
	@echo "" >> $(RESULTS_DIR)/benchmark_quick.txt
	@echo "--- Pthread ---" >> $(RESULTS_DIR)/benchmark_quick.txt
	@time -p $(BUILD_DIR)/matrices_pthread 500 4 2>> $(RESULTS_DIR)/benchmark_quick.txt
	@echo "" >> $(RESULTS_DIR)/benchmark_quick.txt
	@echo "--- Procesos ---" >> $(RESULTS_DIR)/benchmark_quick.txt
	@time -p $(BUILD_DIR)/matrices_procesos 500 4 2>> $(RESULTS_DIR)/benchmark_quick.txt
	@echo "Benchmark rápido completado. Resultados en $(RESULTS_DIR)/benchmark_quick.txt"

# Benchmark de escalabilidad
benchmark-scalability: $(EXECUTABLES)
	@echo "Ejecutando benchmark de escalabilidad..."
	@chmod +x scalability_test.sh
	./scalability_test.sh

# Limpiar archivos compilados
clean:
	rm -rf $(BUILD_DIR) $(RESULTS_DIR)
	rm -f *.gcda *.gcno *.gcov gmon.out
	find . -name "*.o" -delete

# Instalar dependencias (Ubuntu/Debian)
install-deps:
	@echo "Instalando dependencias..."
	sudo apt-get update
	sudo apt-get install -y build-essential gcc g++ make
	sudo apt-get install -y libomp-dev
	sudo apt-get install -y valgrind
	sudo apt-get install -y gprof
	@echo "Dependencias instaladas."

# Verificar sistema
check-system:
	@echo "=== INFORMACIÓN DEL SISTEMA ==="
	@echo "Procesador: $$(lscpu | grep 'Model name' | cut -d: -f2 | xargs)"
	@echo "Núcleos: $$(nproc)"
	@echo "Memoria: $$(free -h | grep 'Mem:' | awk '{print $$2}')"
	@echo "Compilador GCC: $$(gcc --version | head -n1)"
	@echo "OpenMP: $$(gcc -fopenmp -dumpversion 2>/dev/null || echo 'No disponible')"
	@echo "Pthread: $$(ldd /lib/x86_64-linux-gnu/libpthread.so.0 2>/dev/null | head -n1 || echo 'Disponible')"

# Ayuda
help:
	@echo "=== MAKEFILE PARA MULTIPLICACIÓN DE MATRICES OPTIMIZADA ==="
	@echo ""
	@echo "COMANDOS PRINCIPALES:"
	@echo "  make all              - Compilar todas las versiones con O3"
	@echo "  make debug            - Compilar con flags de debug"
	@echo "  make profile          - Compilar con flags de profiling"
	@echo "  make benchmark        - Ejecutar benchmark completo"
	@echo "  make benchmark-quick  - Ejecutar benchmark rápido"
	@echo "  make clean            - Limpiar archivos compilados"
	@echo ""
	@echo "NIVELES DE OPTIMIZACIÓN:"
	@echo "  make optimize-o0      - Compilar con -O0 (sin optimización)"
	@echo "  make optimize-o1      - Compilar con -O1"
	@echo "  make optimize-o2      - Compilar con -O2"
	@echo "  make optimize-fast    - Compilar con optimizaciones agresivas"
	@echo ""
	@echo "UTILIDADES:"
	@echo "  make install-deps     - Instalar dependencias del sistema"
	@echo "  make check-system     - Verificar información del sistema"
	@echo "  make help             - Mostrar esta ayuda"
	@echo ""
	@echo "EJECUTABLES GENERADOS:"
	@echo "  $(BUILD_DIR)/matrices_seq      - Versión secuencial optimizada"
	@echo "  $(BUILD_DIR)/matrices_openmp   - Versión con OpenMP"
	@echo "  $(BUILD_DIR)/matrices_pthread  - Versión con pthread"
	@echo "  $(BUILD_DIR)/matrices_procesos - Versión con procesos"
	@echo ""
	@echo "EJEMPLOS DE USO:"
	@echo "  $(BUILD_DIR)/matrices_seq 1000"
	@echo "  $(BUILD_DIR)/matrices_openmp 1000 4"
	@echo "  $(BUILD_DIR)/matrices_pthread 1000 4"
	@echo "  $(BUILD_DIR)/matrices_procesos 1000 4"

# Regla por defecto
.DEFAULT_GOAL := help
