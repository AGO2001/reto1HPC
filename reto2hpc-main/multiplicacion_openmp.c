#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <omp.h>
#include <chrono>

//multiplicacion_openmp_optimizada.c
// Versión optimizada con OpenMP y mejoras de CPU y memoria

// Función para generar una matriz cuadrada con valores aleatorios (paralelizada)
void generar_matriz_aleatoria_paralela(int **matriz, int n) {
    #pragma omp parallel for collapse(2) schedule(static)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            // Cada hilo usa su propia semilla para evitar condiciones de carrera
            unsigned int seed = omp_get_thread_num() + time(NULL);
            matriz[i][j] = rand_r(&seed) % 100;
        }
    }
}

// Función para multiplicar matrices con OpenMP y optimización de cache
void multiplicar_matrices_openmp_optimizada(int **A, int **B, int **C, int n) {
    const int BLOCK_SIZE = 64; // Tamaño de bloque para optimización de cache
    
    // Inicializar matriz C a cero (paralelizado)
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n; i++) {
        memset(C[i], 0, n * sizeof(int));
    }
    
    // Multiplicación por bloques con OpenMP
    #pragma omp parallel for collapse(2) schedule(dynamic, 1)
    for (int ii = 0; ii < n; ii += BLOCK_SIZE) {
        for (int jj = 0; jj < n; jj += BLOCK_SIZE) {
            for (int kk = 0; kk < n; kk += BLOCK_SIZE) {
                // Calcular límites del bloque
                int i_end = (ii + BLOCK_SIZE < n) ? ii + BLOCK_SIZE : n;
                int j_end = (jj + BLOCK_SIZE < n) ? jj + BLOCK_SIZE : n;
                int k_end = (kk + BLOCK_SIZE < n) ? kk + BLOCK_SIZE : n;
                
                // Multiplicación dentro del bloque
                for (int i = ii; i < i_end; i++) {
                    for (int j = jj; j < j_end; j++) {
                        int sum = C[i][j];
                        for (int k = kk; k < k_end; k++) {
                            sum += A[i][k] * B[k][j];
                        }
                        C[i][j] = sum;
                    }
                }
            }
        }
    }
}

// Función para multiplicar matrices con OpenMP simple (sin blocking)
void multiplicar_matrices_openmp_simple(int **A, int **B, int **C, int n) {
    // Inicializar matriz C a cero (paralelizado)
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n; i++) {
        memset(C[i], 0, n * sizeof(int));
    }
    
    // Multiplicación paralela por filas
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// Función para crear una matriz cuadrada dinámicamente
int **crear_matriz(int n) {
    int **matriz = (int **)malloc(n * sizeof(int *));
    if (matriz == NULL) {
        printf("Error: No se pudo asignar memoria para la matriz\n");
        exit(1);
    }
    
    for (int i = 0; i < n; i++) {
        matriz[i] = (int *)malloc(n * sizeof(int));
        if (matriz[i] == NULL) {
            printf("Error: No se pudo asignar memoria para la fila %d\n", i);
            exit(1);
        }
    }
    
    return matriz;
}

// Función para liberar memoria de una matriz
void liberar_matriz(int **matriz, int n) {
    for (int i = 0; i < n; i++) {
        free(matriz[i]);
    }
    free(matriz);
}

// Función para obtener tiempo en microsegundos
double get_time_microseconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000.0 + tv.tv_usec;
}

// Función para medir uso de memoria (aproximado)
size_t get_memory_usage() {
    FILE* file = fopen("/proc/self/status", "r");
    if (file == NULL) return 0;
    
    char line[128];
    size_t memory = 0;
    
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line, "VmRSS: %zu kB", &memory);
            break;
        }
    }
    fclose(file);
    return memory;
}

int main(int argc, char *argv[]) {
    // Verificar argumentos de línea de comandos
    if (argc != 3) {
        printf("Uso: %s <tamaño_matriz> <num_hilos>\n", argv[0]);
        printf("Ejemplo: %s 1000 4\n", argv[0]);
        return 1;
    }
    
    // Convertir argumentos
    int n = atoi(argv[1]);
    int num_hilos = atoi(argv[2]);
    
    // Verificar que los argumentos sean válidos
    if (n <= 0 || num_hilos <= 0) {
        printf("Error: El tamaño de matriz y número de hilos deben ser positivos\n");
        return 1;
    }
    
    // Configurar número de hilos de OpenMP
    omp_set_num_threads(num_hilos);
    
    printf("=== MULTIPLICACIÓN DE MATRICES CON OPENMP OPTIMIZADA ===\n");
    printf("Tamaño de matriz: %dx%d\n", n, n);
    printf("Número de hilos: %d\n", num_hilos);
    printf("Hilos disponibles: %d\n", omp_get_max_threads());
    printf("Memoria inicial: %zu kB\n\n", get_memory_usage());
    
    // Inicializar generador de números aleatorios
    srand(time(NULL));
    
    // Crear las tres matrices: A, B y C (resultado)
    int **matriz_A = crear_matriz(n);
    int **matriz_B = crear_matriz(n);
    int **matriz_C = crear_matriz(n);
    
    printf("Memoria después de crear matrices: %zu kB\n", get_memory_usage());
    
    // Generar matrices A y B con valores aleatorios (paralelizado)
    double start_gen = get_time_microseconds();
    generar_matriz_aleatoria_paralela(matriz_A, n);
    generar_matriz_aleatoria_paralela(matriz_B, n);
    double end_gen = get_time_microseconds();
    
    printf("Tiempo de generación de matrices: %.2f microsegundos\n", end_gen - start_gen);
    printf("Memoria después de generar matrices: %zu kB\n\n", get_memory_usage());

    // Prueba con algoritmo OpenMP simple
    printf("--- ALGORITMO OPENMP SIMPLE ---\n");
    auto start_simple = std::chrono::high_resolution_clock::now();
    multiplicar_matrices_openmp_simple(matriz_A, matriz_B, matriz_C, n);
    auto end_simple = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_simple = end_simple - start_simple;
    printf("Tiempo de multiplicación OpenMP simple: %f segundos\n", duration_simple.count());
    printf("Memoria durante multiplicación simple: %zu kB\n\n", get_memory_usage());

    // Limpiar matriz C para la siguiente prueba
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n; i++) {
        memset(matriz_C[i], 0, n * sizeof(int));
    }

    // Prueba con algoritmo OpenMP optimizado
    printf("--- ALGORITMO OPENMP OPTIMIZADO ---\n");
    auto start_opt = std::chrono::high_resolution_clock::now();
    multiplicar_matrices_openmp_optimizada(matriz_A, matriz_B, matriz_C, n);
    auto end_opt = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_opt = end_opt - start_opt;
    printf("Tiempo de multiplicación OpenMP optimizada: %f segundos\n", duration_opt.count());
    printf("Memoria durante multiplicación optimizada: %zu kB\n\n", get_memory_usage());

    // Calcular speedup y eficiencia
    double speedup = duration_simple.count() / duration_opt.count();
    double eficiencia = speedup / num_hilos;
    
    printf("=== RESULTADOS DE BENCHMARK ===\n");
    printf("Speedup: %.2fx\n", speedup);
    printf("Eficiencia: %.2f%%\n", eficiencia * 100);
    printf("Mejora de rendimiento: %.1f%%\n", ((duration_simple.count() - duration_opt.count()) / duration_simple.count()) * 100);
    printf("Memoria final: %zu kB\n", get_memory_usage());

    // Liberar memoria
    liberar_matriz(matriz_A, n);
    liberar_matriz(matriz_B, n);
    liberar_matriz(matriz_C, n);
    
    printf("=== PROGRAMA COMPLETADO EXITOSAMENTE ===\n");
    return 0;
}
