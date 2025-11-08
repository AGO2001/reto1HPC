#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h> // Para fork()
#include <sys/mman.h> // Para mmap
#include <sys/wait.h> // Para wait
#include <chrono>
#include <string.h>
#include <sys/time.h>

//multiplicacion_procesos_optimizada.c
// Versión optimizada con mejoras de CPU, memoria y cache

// Estructura para pasar datos a los procesos
typedef struct {
    int **matriz;
    int n;
    int proc_id;
    char nombre;
} DatosMatriz;

// Multiplicación de matrices por bloques de filas (optimizada para procesos)
// C es un arreglo 1D en memoria compartida
void multiplicar_matrices_proceso_optimizada(int **A, int **B, int *C, int n, int fila_inicio, int fila_fin, int proc_id) {
    const int BLOCK_SIZE = 32; // Tamaño de bloque para optimización de cache
    
    printf("Proceso %d: Procesando filas %d a %d (optimizado)\n", proc_id, fila_inicio, fila_fin - 1);
    
    // Inicializar bloque de filas a cero
    for (int i = fila_inicio; i < fila_fin; i++) {
        for (int j = 0; j < n; j++) {
            C[i * n + j] = 0;
        }
    }
    
    // Multiplicación por bloques para mejor uso de cache
    for (int kk = 0; kk < n; kk += BLOCK_SIZE) {
        int k_end = (kk + BLOCK_SIZE < n) ? kk + BLOCK_SIZE : n;
        
        for (int jj = 0; jj < n; jj += BLOCK_SIZE) {
            int j_end = (jj + BLOCK_SIZE < n) ? jj + BLOCK_SIZE : n;
            
            for (int i = fila_inicio; i < fila_fin; i++) {
                for (int j = jj; j < j_end; j++) {
                    int sum = C[i * n + j];
                    for (int k = kk; k < k_end; k++) {
                        sum += A[i][k] * B[k][j];
                    }
                    C[i * n + j] = sum;
                }
            }
        }
    }
    
    printf("Proceso %d: Filas %d a %d completadas (optimizado).\n", proc_id, fila_inicio, fila_fin - 1);
}

// Multiplicación de matrices por bloques de filas (versión original)
void multiplicar_matrices_proceso_original(int **A, int **B, int *C, int n, int fila_inicio, int fila_fin, int proc_id) {
    printf("Proceso %d: Procesando filas %d a %d (original)\n", proc_id, fila_inicio, fila_fin - 1);
    for (int i = fila_inicio; i < fila_fin; i++) {
        for (int j = 0; j < n; j++) {
            C[i * n + j] = 0;
            for (int k = 0; k < n; k++) {
                C[i * n + j] += A[i][k] * B[k][j];
            }
        }
    }
    printf("Proceso %d: Filas %d a %d completadas (original).\n", proc_id, fila_inicio, fila_fin - 1);
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
        printf("Uso: %s <tamaño_matriz> <num_procesos>\n", argv[0]);
        printf("Ejemplo: %s 1000 4\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[1]);
    int num_procesos = atoi(argv[2]);
    if (n <= 0 || num_procesos <= 0) {
        printf("Error: El tamaño de matriz y número de procesos deben ser positivos\n");
        return 1;
    }
    if (num_procesos > n) {
        num_procesos = n;
        printf("Ajustando número de procesos a %d (máximo: tamaño de matriz)\n", n);
    }
    printf("=== MULTIPLICACIÓN DE MATRICES CON PROCESOS OPTIMIZADA ===\n");
    printf("Tamaño de matriz: %dx%d\n", n, n);
    printf("Procesos para multiplicación: %d\n", num_procesos);
    printf("Memoria inicial: %zu kB\n\n", get_memory_usage());

    // Crear matrices A y B en memoria normal
    int **matriz_A = crear_matriz(n);
    int **matriz_B = crear_matriz(n);
    // Crear matriz C en memoria compartida (1D)
    int *matriz_C = (int *)mmap(NULL, n * n * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (matriz_C == MAP_FAILED) {
        printf("Error: No se pudo asignar memoria compartida para la matriz C\n");
        exit(1);
    }
    
    printf("Memoria después de crear matrices: %zu kB\n", get_memory_usage());

    // Generar matrices A y B con valores aleatorios (secuencial)
    double start_gen = get_time_microseconds();
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matriz_A[i][j] = rand() % 100;
            matriz_B[i][j] = rand() % 100;
        }
    }
    double end_gen = get_time_microseconds();
    
    printf("Tiempo de generación de matrices: %.2f microsegundos\n", end_gen - start_gen);
    printf("Memoria después de generar matrices: %zu kB\n\n", get_memory_usage());

    // Calcular filas por proceso
    int filas_por_proceso = n / num_procesos;
    int filas_restantes = n % num_procesos;

    // Prueba con algoritmo original
    printf("--- ALGORITMO PROCESOS ORIGINAL ---\n");
    auto start_orig = std::chrono::high_resolution_clock::now();
    int fila_inicio = 0;
    for (int i = 0; i < num_procesos; i++) {
        int fila_fin = fila_inicio + filas_por_proceso;
        if (i < filas_restantes) fila_fin++;
        pid_t pid = fork();
        if (pid == 0) {
            // Proceso hijo: multiplica su bloque de filas
            multiplicar_matrices_proceso_original(matriz_A, matriz_B, matriz_C, n, fila_inicio, fila_fin, i);
            // Liberar memoria en hijo
            liberar_matriz(matriz_A, n);
            liberar_matriz(matriz_B, n);
            munmap(matriz_C, n * n * sizeof(int));
            exit(0);
        }
        // Proceso padre: avanza al siguiente bloque
        fila_inicio = fila_fin;
    }
    // Esperar a que todos los hijos terminen
    for (int i = 0; i < num_procesos; i++) {
        wait(NULL);
    }
    auto end_orig = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_orig = end_orig - start_orig;
    printf("Tiempo de multiplicación original: %f segundos\n", duration_orig.count());
    printf("Memoria durante multiplicación original: %zu kB\n\n", get_memory_usage());

    // Limpiar matriz C para la siguiente prueba
    memset(matriz_C, 0, n * n * sizeof(int));

    // Prueba con algoritmo optimizado
    printf("--- ALGORITMO PROCESOS OPTIMIZADO ---\n");
    auto start_opt = std::chrono::high_resolution_clock::now();
    fila_inicio = 0;
    for (int i = 0; i < num_procesos; i++) {
        int fila_fin = fila_inicio + filas_por_proceso;
        if (i < filas_restantes) fila_fin++;
        pid_t pid = fork();
        if (pid == 0) {
            // Proceso hijo: multiplica su bloque de filas
            multiplicar_matrices_proceso_optimizada(matriz_A, matriz_B, matriz_C, n, fila_inicio, fila_fin, i);
            // Liberar memoria en hijo
            liberar_matriz(matriz_A, n);
            liberar_matriz(matriz_B, n);
            munmap(matriz_C, n * n * sizeof(int));
            exit(0);
        }
        // Proceso padre: avanza al siguiente bloque
        fila_inicio = fila_fin;
    }
    // Esperar a que todos los hijos terminen
    for (int i = 0; i < num_procesos; i++) {
        wait(NULL);
    }
    auto end_opt = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_opt = end_opt - start_opt;
    printf("Tiempo de multiplicación optimizada: %f segundos\n", duration_opt.count());
    printf("Memoria durante multiplicación optimizada: %zu kB\n\n", get_memory_usage());

    // Calcular speedup y eficiencia
    double speedup = duration_orig.count() / duration_opt.count();
    double eficiencia = speedup / num_procesos;
    
    printf("=== RESULTADOS DE BENCHMARK ===\n");
    printf("Speedup: %.2fx\n", speedup);
    printf("Eficiencia: %.2f%%\n", eficiencia * 100);
    printf("Mejora de rendimiento: %.1f%%\n", ((duration_orig.count() - duration_opt.count()) / duration_orig.count()) * 100);
    printf("Memoria final: %zu kB\n", get_memory_usage());

    // Liberar memoria en padre
    liberar_matriz(matriz_A, n);
    liberar_matriz(matriz_B, n);
    munmap(matriz_C, n * n * sizeof(int));
    printf("=== PROGRAMA COMPLETADO EXITOSAMENTE ===\n");
    return 0;
}
// Nota: Se eliminó todo lo relacionado con pthread y mutex, y la generación de matrices es secuencial.