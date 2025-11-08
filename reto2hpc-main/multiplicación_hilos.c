#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <chrono>
#include <string.h>
#include <sys/time.h>

//multiplicacion_hilos_optimizada.c
// Versión optimizada con mejoras de CPU, memoria y cache

// Estructura para pasar datos a los hilos
typedef struct {
    int **matriz;
    int n;
    int hilo_id;
    char nombre;
} DatosMatriz;

// Estructura para hilos de multiplicación optimizada
typedef struct {
    int **A;
    int **B;
    int **C;
    int n;
    int fila_inicio;
    int fila_fin;
    int hilo_id;
} DatosMultiplicacion;

// Variables globales para sincronización
pthread_mutex_t mutex_print = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier_generacion;
pthread_barrier_t barrier_multiplicacion;

// Función para generar una matriz cuadrada con valores aleatorios (versión hilo)
void* generar_matriz_aleatoria_hilo(void* arg) {
    DatosMatriz* datos = (DatosMatriz*)arg;
    
    // Usar semilla diferente para cada hilo basada en tiempo + hilo_id
    unsigned int semilla = time(NULL) + datos->hilo_id * 1000;
    
    pthread_mutex_lock(&mutex_print);
    printf("Hilo %d: Generando matriz %c (%dx%d)...\n", 
           datos->hilo_id, datos->nombre, datos->n, datos->n);
    pthread_mutex_unlock(&mutex_print);
    
    for (int i = 0; i < datos->n; i++) {
        for (int j = 0; j < datos->n; j++) {
            datos->matriz[i][j] = rand_r(&semilla) % 100;
        }
    }
    
    pthread_mutex_lock(&mutex_print);
    printf("Hilo %d: Matriz %c completada.\n", datos->hilo_id, datos->nombre);
    pthread_mutex_unlock(&mutex_print);
    
    pthread_exit(NULL);
}

// Función para multiplicar matrices por bloques de filas (optimizada)
void* multiplicar_matrices_hilo_optimizada(void* arg) {
    DatosMultiplicacion* datos = (DatosMultiplicacion*)arg;
    const int BLOCK_SIZE = 32; // Tamaño de bloque para optimización de cache
    
    pthread_mutex_lock(&mutex_print);
    printf("Hilo %d: Procesando filas %d a %d (optimizado)\n", 
           datos->hilo_id, datos->fila_inicio, datos->fila_fin - 1);
    pthread_mutex_unlock(&mutex_print);
    
    // Inicializar bloque de filas a cero
    for (int i = datos->fila_inicio; i < datos->fila_fin; i++) {
        memset(datos->C[i], 0, datos->n * sizeof(int));
    }
    
    // Multiplicación por bloques para mejor uso de cache
    for (int kk = 0; kk < datos->n; kk += BLOCK_SIZE) {
        int k_end = (kk + BLOCK_SIZE < datos->n) ? kk + BLOCK_SIZE : datos->n;
        
        for (int jj = 0; jj < datos->n; jj += BLOCK_SIZE) {
            int j_end = (jj + BLOCK_SIZE < datos->n) ? jj + BLOCK_SIZE : datos->n;
            
            for (int i = datos->fila_inicio; i < datos->fila_fin; i++) {
                for (int j = jj; j < j_end; j++) {
                    int sum = datos->C[i][j];
                    for (int k = kk; k < k_end; k++) {
                        sum += datos->A[i][k] * datos->B[k][j];
                    }
                    datos->C[i][j] = sum;
                }
            }
        }
    }
    
    pthread_mutex_lock(&mutex_print);
    printf("Hilo %d: Filas %d a %d completadas (optimizado).\n", 
           datos->hilo_id, datos->fila_inicio, datos->fila_fin - 1);
    pthread_mutex_unlock(&mutex_print);
    
    pthread_exit(NULL);
}

// Función para multiplicar matrices por bloques de filas (versión original)
void* multiplicar_matrices_hilo_original(void* arg) {
    DatosMultiplicacion* datos = (DatosMultiplicacion*)arg;
    
    pthread_mutex_lock(&mutex_print);
    printf("Hilo %d: Procesando filas %d a %d (original)\n", 
           datos->hilo_id, datos->fila_inicio, datos->fila_fin - 1);
    pthread_mutex_unlock(&mutex_print);
    
    for (int i = datos->fila_inicio; i < datos->fila_fin; i++) {
        for (int j = 0; j < datos->n; j++) {
            datos->C[i][j] = 0;
            for (int k = 0; k < datos->n; k++) {
                datos->C[i][j] += datos->A[i][k] * datos->B[k][j];
            }
        }
    }
    
    pthread_mutex_lock(&mutex_print);
    printf("Hilo %d: Filas %d a %d completadas (original).\n", 
           datos->hilo_id, datos->fila_inicio, datos->fila_fin - 1);
    pthread_mutex_unlock(&mutex_print);
    
    pthread_exit(NULL);
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
        printf("Uso: %s <tamaño_matriz> <num_hilos_multiplicacion>\n", argv[0]);
        printf("Ejemplo: %s 1000 4\n", argv[0]);
        return 1;
    }
    
    // Convertir argumentos
    int n = atoi(argv[1]);
    int num_hilos_mult = atoi(argv[2]);
    
    // Verificar que los argumentos sean válidos
    if (n <= 0 || num_hilos_mult <= 0) {
        printf("Error: El tamaño de matriz y número de hilos deben ser positivos\n");
        return 1;
    }
    
    if (num_hilos_mult > n) {
        num_hilos_mult = n;
        printf("Ajustando número de hilos de multiplicación a %d (máximo: tamaño de matriz)\n", n);
    }
    
    printf("=== MULTIPLICACIÓN DE MATRICES CON HILOS OPTIMIZADA ===\n");
    printf("Tamaño de matriz: %dx%d\n", n, n);
    printf("Hilos para multiplicación: %d\n", num_hilos_mult);
    printf("Memoria inicial: %zu kB\n\n", get_memory_usage());

    // Inicializar barreras
    pthread_barrier_init(&barrier_generacion, NULL, 2);
    pthread_barrier_init(&barrier_multiplicacion, NULL, num_hilos_mult);

    int **matriz_A = crear_matriz(n);
    int **matriz_B = crear_matriz(n);
    int **matriz_C = crear_matriz(n);
    
    printf("Memoria después de crear matrices: %zu kB\n", get_memory_usage());
    
    pthread_t hilo_A, hilo_B;
    DatosMatriz datos_A = {matriz_A, n, 1, 'A'};
    DatosMatriz datos_B = {matriz_B, n, 2, 'B'};

    // Crear hilos para generar matrices A y B simultáneamente
    double start_gen = get_time_microseconds();
    pthread_create(&hilo_A, NULL, generar_matriz_aleatoria_hilo, &datos_A);
    pthread_create(&hilo_B, NULL, generar_matriz_aleatoria_hilo, &datos_B);
    pthread_join(hilo_A, NULL);
    pthread_join(hilo_B, NULL);
    double end_gen = get_time_microseconds();
    
    printf("Tiempo de generación de matrices: %.2f microsegundos\n", end_gen - start_gen);
    printf("Memoria después de generar matrices: %zu kB\n\n", get_memory_usage());

    pthread_t hilos_mult[num_hilos_mult];
    DatosMultiplicacion datos_mult[num_hilos_mult];
    int filas_por_hilo = n / num_hilos_mult;
    int filas_restantes = n % num_hilos_mult;

    // Configurar datos para hilos
    int fila_inicio = 0;
    for (int i = 0; i < num_hilos_mult; i++) {
        datos_mult[i].A = matriz_A;
        datos_mult[i].B = matriz_B;
        datos_mult[i].C = matriz_C;
        datos_mult[i].n = n;
        datos_mult[i].hilo_id = i;
        datos_mult[i].fila_inicio = fila_inicio;
        datos_mult[i].fila_fin = fila_inicio + filas_por_hilo;
        if (i < filas_restantes) datos_mult[i].fila_fin++;
        fila_inicio = datos_mult[i].fila_fin;
    }

    // Prueba con algoritmo original
    printf("--- ALGORITMO PTHREAD ORIGINAL ---\n");
    auto start_orig = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_hilos_mult; i++) {
        pthread_create(&hilos_mult[i], NULL, multiplicar_matrices_hilo_original, &datos_mult[i]);
    }
    for (int i = 0; i < num_hilos_mult; i++) {
        pthread_join(hilos_mult[i], NULL);
    }
    auto end_orig = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_orig = end_orig - start_orig;
    printf("Tiempo de multiplicación original: %f segundos\n", duration_orig.count());
    printf("Memoria durante multiplicación original: %zu kB\n\n", get_memory_usage());

    // Limpiar matriz C para la siguiente prueba
    for (int i = 0; i < n; i++) {
        memset(matriz_C[i], 0, n * sizeof(int));
    }

    // Prueba con algoritmo optimizado
    printf("--- ALGORITMO PTHREAD OPTIMIZADO ---\n");
    auto start_opt = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_hilos_mult; i++) {
        pthread_create(&hilos_mult[i], NULL, multiplicar_matrices_hilo_optimizada, &datos_mult[i]);
    }
    for (int i = 0; i < num_hilos_mult; i++) {
        pthread_join(hilos_mult[i], NULL);
    }
    auto end_opt = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_opt = end_opt - start_opt;
    printf("Tiempo de multiplicación optimizada: %f segundos\n", duration_opt.count());
    printf("Memoria durante multiplicación optimizada: %zu kB\n\n", get_memory_usage());

    // Calcular speedup y eficiencia
    double speedup = duration_orig.count() / duration_opt.count();
    double eficiencia = speedup / num_hilos_mult;
    
    printf("=== RESULTADOS DE BENCHMARK ===\n");
    printf("Speedup: %.2fx\n", speedup);
    printf("Eficiencia: %.2f%%\n", eficiencia * 100);
    printf("Mejora de rendimiento: %.1f%%\n", ((duration_orig.count() - duration_opt.count()) / duration_orig.count()) * 100);
    printf("Memoria final: %zu kB\n", get_memory_usage());

    // Liberar recursos
    liberar_matriz(matriz_A, n);
    liberar_matriz(matriz_B, n);
    liberar_matriz(matriz_C, n);
    pthread_mutex_destroy(&mutex_print);
    pthread_barrier_destroy(&barrier_generacion);
    pthread_barrier_destroy(&barrier_multiplicacion);
    
    printf("=== PROGRAMA COMPLETADO EXITOSAMENTE ===\n");
    return 0;
}