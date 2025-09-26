#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <time.h>

double dartboard_method_processes(int num_points, int num_processes) {
    // Memoria compartida para contador
    int* shared_counter = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                              MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *shared_counter = 0;
    
    int points_per_process = num_points / num_processes;
    pid_t pids[num_processes];
    
    for (int p = 0; p < num_processes; p++) {
        pid_t pid = fork();
        
        if (pid == 0) { // Proceso hijo
            srand(time(NULL) + getpid());
            int local_inside = 0;
            
            for (int i = 0; i < points_per_process; i++) {
                double x = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
                double y = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
                
                if (x * x + y * y <= 1.0) {
                    local_inside++;
                }
            }
            
            // Actualizar contador compartido (requiere sincronización)
            __atomic_add_fetch(shared_counter, local_inside, __ATOMIC_SEQ_CST);
            exit(0);
            
        } else if (pid > 0) { // Proceso padre
            pids[p] = pid;
        } else {
            perror("fork failed");
            exit(1);
        }
    }
    
    // Esperar todos los procesos hijo
    for (int p = 0; p < num_processes; p++) {
        waitpid(pids[p], NULL, 0);
    }
    
    double pi_estimate = 4.0 * (*shared_counter) / num_points;
    munmap(shared_counter, sizeof(int));
    
    return pi_estimate;
}

int main() {
    int num_points = 1000000;
    int num_processes = 4; // Número de procesos

    clock_t start = clock();
    double pi_estimate = dartboard_method_processes(num_points, num_processes);
    clock_t end = clock();

    double execution_time = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("Dartboard Method - Procesos\n");
    printf("Puntos: %d\n", num_points);
    printf("Procesos: %d\n", num_processes);
    printf("Estimación de π: %.6f\n", pi_estimate);
    printf("Error: %.6f\n", fabs(M_PI - pi_estimate));
    printf("Tiempo de ejecución: %.4f segundos\n", execution_time);

    return 0;
}
