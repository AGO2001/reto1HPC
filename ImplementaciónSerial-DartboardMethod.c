#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

double dartboard_method_serial(int num_points) {
    srand(time(NULL));
    int inside_circle = 0;
    
    for (int i = 0; i < num_points; i++) {
        // Generar coordenadas aleatorias [-1, 1]
        double x = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
        double y = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
        
        // Verificar si está dentro del círculo unitario
        if (x * x + y * y <= 1.0) {
            inside_circle++;
        }
    }
    
    // Estimar π
    return 4.0 * inside_circle / num_points;
}

int main() {
    int num_points = 1000000;
    
    clock_t start = clock();
    double pi_estimate = dartboard_method_serial(num_points);
    clock_t end = clock();
    
    double execution_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Dartboard Method - Serial\n");
    printf("Puntos: %d\n", num_points);
    printf("Estimación de π: %.6f\n", pi_estimate);
    printf("Error: %.6f\n", fabs(M_PI - pi_estimate));
    printf("Tiempo de ejecución: %.4f segundos\n", execution_time);
    
    return 0;
}
