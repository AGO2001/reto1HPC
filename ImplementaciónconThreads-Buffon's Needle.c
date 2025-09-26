#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    int thread_id;
    int tosses_per_thread;
    double needle_length;
    double line_distance;
    int crosses;
} thread_data_t;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int total_crosses = 0;

void* buffon_thread(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    unsigned int seed = time(NULL) + data->thread_id;
    int local_crosses = 0;
    
    for (int i = 0; i < data->tosses_per_thread; i++) {
        double x = ((double)rand_r(&seed) / RAND_MAX) * data->line_distance;
        double theta = ((double)rand_r(&seed) / RAND_MAX) * M_PI;
        
        double x_left = x - (data->needle_length / 2.0) * sin(theta);
        double x_right = x + (data->needle_length / 2.0) * sin(theta);
        
        if (x_left < 0.0 || x_right > data->line_distance) {
            local_crosses++;
        }
    }
    
    pthread_mutex_lock(&mutex);
    total_crosses += local_crosses;
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}

double buffon_needle_threads(int num_tosses, int num_threads, 
                           double needle_length, double line_distance) {
    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    thread_data_t* thread_data = malloc(num_threads * sizeof(thread_data_t));
    
    int tosses_per_thread = num_tosses / num_threads;
    total_crosses = 0;
    
    // Crear threads
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].tosses_per_thread = tosses_per_thread;
        thread_data[i].needle_length = needle_length;
        thread_data[i].line_distance = line_distance;
        
        pthread_create(&threads[i], NULL, buffon_thread, &thread_data[i]);
    }
    
    // Esperar threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    free(threads);
    free(thread_data);
    
    double probability = (double)total_crosses / num_tosses;
    return (2.0 * needle_length) / (probability * line_distance);
}

int main() {
    int num_tosses = 1000000;
    int num_threads = 4; // Número de hilos
    double needle_length = 1.0;
    double line_distance = 2.0;

    clock_t start = clock();
    double pi_estimate = buffon_needle_threads(num_tosses, num_threads, needle_length, line_distance);
    clock_t end = clock();

    double execution_time = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("Buffon's Needle - Threads\n");
    printf("Lanzamientos: %d\n", num_tosses);
    printf("Hilos: %d\n", num_threads);
    printf("Estimación de π: %.6f\n", pi_estimate);
    printf("Error: %.6f\n", fabs(M_PI - pi_estimate));
    printf("Tiempo de ejecución: %.4f segundos\n", execution_time);

    return 0;
}
