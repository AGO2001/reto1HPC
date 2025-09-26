#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

double buffon_needle_serial(int num_tosses, double needle_length, double line_distance) {
    srand(time(NULL));
    int crosses = 0;
    
    for (int i = 0; i < num_tosses; i++) {
        // Generar posición x aleatoria [0, line_distance]
        double x = ((double)rand() / RAND_MAX) * line_distance;
        
        // Generar ángulo θ aleatorio [0, π]
        double theta = ((double)rand() / RAND_MAX) * M_PI;
        
        // Calcular extremos de la aguja
        double x_left = x - (needle_length / 2.0) * sin(theta);
        double x_right = x + (needle_length / 2.0) * sin(theta);
        
        // Verificar si cruza una línea
        if (x_left < 0.0 || x_right > line_distance) {
            crosses++;
        }
    }
    
    // Calcular π usando la fórmula de Buffon
    double probability = (double)crosses / num_tosses;
    return (2.0 * needle_length) / (probability * line_distance);
}

int main() {
    int num_tosses = 1000000;
    double needle_length = 1.0;
    double line_distance = 2.0;
    
    clock_t start = clock();
    double pi_estimate = buffon_needle_serial(num_tosses, needle_length, line_distance);
    clock_t end = clock();
    
    double execution_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Buffon's Needle - Serial\n");
    printf("Lanzamientos: %d\n", num_tosses);
    printf("Estimación de π: %.6f\n", pi_estimate);
    printf("Error: %.6f\n", fabs(M_PI - pi_estimate));
    printf("Tiempo de ejecución: %.4f segundos\n", execution_time);
    
    return 0;
}
