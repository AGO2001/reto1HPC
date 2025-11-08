#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <chrono>
#include <string.h>
#include <sys/time.h>

//multiplicacion_matrices_optimizada.c
// Versión optimizada con mejoras de CPU y memoria

// Función para generar una matriz cuadrada con valores aleatorios (optimizada)
void generar_matriz_aleatoria(int **matriz, int n) {
    // Usar un solo bucle para mejor localidad de cache
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matriz[i][j] = rand() % 100; // Números aleatorios del 0 al 99
        }
    }
}

// Función para multiplicar matrices con optimización de cache (blocking/tiling)
void multiplicar_matrices_optimizada(int **A, int **B, int **C, int n) {
    const int BLOCK_SIZE = 64; // Tamaño de bloque para optimización de cache
    
    // Inicializar matriz C a cero
    for (int i = 0; i < n; i++) {
        memset(C[i], 0, n * sizeof(int));
    }
    
    // Multiplicación por bloques para mejor uso de cache
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

// Función original para comparación
void multiplicar_matrices_original(int **A, int **B, int **C, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            C[i][j] = 0;
            for (int k = 0; k < n; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// Función para guardar una matriz en un archivo de texto

// Función para crear una matriz cuadrada dinámicamente
int **crear_matriz(int n) {
    int **matriz = (int **)malloc(n * sizeof(int *));
    if (matriz == NULL) {
        exit(1);
    }
    
    for (int i = 0; i < n; i++) {
        matriz[i] = (int *)malloc(n * sizeof(int));
        if (matriz[i] == NULL) {
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
    if (argc != 2) {
        printf("Uso: %s <tamaño_matriz>\n", argv[0]);
        printf("Ejemplo: %s 1000\n", argv[0]);
        return 1;
    }
    
    // Convertir argumento a entero (tamaño de la matriz cuadrada)
    int n = atoi(argv[1]);
    
    // Verificar que el tamaño sea válido
    if (n <= 0) {
        printf("Error: El tamaño de matriz debe ser positivo\n");
        return 1;
    }
    
    printf("=== MULTIPLICACIÓN DE MATRICES OPTIMIZADA ===\n");
    printf("Tamaño de matriz: %dx%d\n", n, n);
    printf("Memoria inicial: %zu kB\n\n", get_memory_usage());
    
    // Inicializar generador de números aleatorios
    srand(time(NULL));
    
    // Crear las tres matrices: A, B y C (resultado)
    int **matriz_A = crear_matriz(n);
    int **matriz_B = crear_matriz(n);
    int **matriz_C = crear_matriz(n);
    
    printf("Memoria después de crear matrices: %zu kB\n", get_memory_usage());
    
    // Generar matrices A y B con valores aleatorios
    double start_gen = get_time_microseconds();
    generar_matriz_aleatoria(matriz_A, n);
    generar_matriz_aleatoria(matriz_B, n);
    double end_gen = get_time_microseconds();
    
    printf("Tiempo de generación de matrices: %.2f microsegundos\n", end_gen - start_gen);
    printf("Memoria después de generar matrices: %zu kB\n\n", get_memory_usage());

    // Prueba con algoritmo original
    printf("--- ALGORITMO ORIGINAL ---\n");
    auto start_orig = std::chrono::high_resolution_clock::now();
    multiplicar_matrices_original(matriz_A, matriz_B, matriz_C, n);
    auto end_orig = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_orig = end_orig - start_orig;
    printf("Tiempo de multiplicación original: %f segundos\n", duration_orig.count());
    printf("Memoria durante multiplicación original: %zu kB\n\n", get_memory_usage());

    // Limpiar matriz C para la siguiente prueba
    for (int i = 0; i < n; i++) {
        memset(matriz_C[i], 0, n * sizeof(int));
    }

    // Prueba con algoritmo optimizado
    printf("--- ALGORITMO OPTIMIZADO ---\n");
    auto start_opt = std::chrono::high_resolution_clock::now();
    multiplicar_matrices_optimizada(matriz_A, matriz_B, matriz_C, n);
    auto end_opt = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_opt = end_opt - start_opt;
    printf("Tiempo de multiplicación optimizada: %f segundos\n", duration_opt.count());
    printf("Memoria durante multiplicación optimizada: %zu kB\n\n", get_memory_usage());

    // Calcular speedup
    double speedup = duration_orig.count() / duration_opt.count();
    printf("=== RESULTADOS DE BENCHMARK ===\n");
    printf("Speedup: %.2fx\n", speedup);
    printf("Mejora de rendimiento: %.1f%%\n", ((duration_orig.count() - duration_opt.count()) / duration_orig.count()) * 100);
    printf("Memoria final: %zu kB\n", get_memory_usage());

    // Liberar memoria
    liberar_matriz(matriz_A, n);
    liberar_matriz(matriz_B, n);
    liberar_matriz(matriz_C, n);
    
    printf("=== PROGRAMA COMPLETADO EXITOSAMENTE ===\n");
    return 0;
}
