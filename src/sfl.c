#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

/* ===================== CONSTANTES ===================== */

#define X_MAX   5.12     // límite superior del espacio de búsqueda
#define X_MIN  -5.12     // límite inferior del espacio de búsqueda

#define P  20            // número de ranas (población total)
#define R   3            // dimensiones del problema
#define M   5            // número de memeplexes (divisiones)
#define J_MAX  5         // pasos de evolución dentro de cada memeplex

#define MAX_ITERS 100    // número de iteraciones (barajados globales)
#define D_MAX  ((X_MAX - X_MIN) * 0.25)  // salto máximo permitido por componente

/* ===================== VARIABLES GLOBALES ===================== */

double x[P][R];      // posición de cada rana
double fit[P];       // fitness de cada rana

double g[R];         // mejor posición global
double g_fitness;    // fitness de la mejor global

/* ===================== DECLARACIONES ===================== */

void inicializar_poblacion(void);
void evaluar_fitness_poblacion(double xx[P][R], double f[P]);
double fitness(double vec[R]);

void ordenar_por_fitness(double xx[P][R], double f[P], int n);
void copiar_vec(double dst[R], const double src[R]);
void clamp_en_rango(double *v);
void reinit_aleatoria(double *v);

void dividir_en_memeplexes(int mem_idx[M][P], int mem_sz[M]);
void procesar_memeplex(int mem_vec[], int mem_n);
void intentar_mejorar_peor(int idx_best, int idx_worst);
void actualizar_mejor_global_posicion(int idx);

void imprimir_progreso(int iter);

/* ===================== IMPLEMENTACIÓN ===================== */

void inicializar_poblacion(void) {
    for (int i = 0; i < P; ++i) {
        for (int d = 0; d < R; ++d) {
            x[i][d] = X_MIN + (X_MAX - X_MIN) * drand48();
        }
    }
    evaluar_fitness_poblacion(x, fit);

    int best = 0;
    for (int i = 1; i < P; ++i) if (fit[i] < fit[best]) best = i;
    copiar_vec(g, x[best]);
    g_fitness = fit[best];
}

void evaluar_fitness_poblacion(double xx[P][R], double f[P]) {
    for (int i = 0; i < P; ++i) f[i] = fitness(xx[i]);
}

// Sphere = sum(x_i^2)
double fitness(double vec[R]) {
    double s = 0.0;
    for (int d = 0; d < R; ++d) s += vec[d] * vec[d];
    return s;
}

// ordenación simple por fitness ascendente (x y fit en paralelo)
void ordenar_por_fitness(double xx[P][R], double f[P], int n) {
    for (int i = 0; i < n - 1; ++i) {
        int minj = i;
        for (int j = i + 1; j < n; ++j) {
            if (f[j] < f[minj]) minj = j;
        }
        if (minj != i) {
            double tmpf = f[i]; f[i] = f[minj]; f[minj] = tmpf;
            for (int d = 0; d < R; ++d) {
                double t = xx[i][d];
                xx[i][d]    = xx[minj][d];
                xx[minj][d] = t;
            }
        }
    }
}

void copiar_vec(double dst[R], const double src[R]) {
    for (int d = 0; d < R; ++d) dst[d] = src[d];
}

void clamp_en_rango(double *v) {
    for (int d = 0; d < R; ++d) {
        if (v[d] < X_MIN) v[d] = X_MIN;
        if (v[d] > X_MAX) v[d] = X_MAX;
    }
}

void reinit_aleatoria(double *v) {
    for (int d = 0; d < R; ++d) v[d] = X_MIN + (X_MAX - X_MIN) * drand48();
}

void dividir_en_memeplexes(int mem_idx[M][P], int mem_sz[M]) {
    for (int m = 0; m < M; ++m) mem_sz[m] = 0;
    for (int i = 0; i < P; ++i) {
        int m = i % M; // round-robin
        mem_idx[m][mem_sz[m]++] = i;
    }
}

void procesar_memeplex(int mem_vec[], int mem_n) {
    for (int step = 0; step < J_MAX; ++step) {
        int idx_best = mem_vec[0];
        int idx_worst = mem_vec[0];
        for (int k = 1; k < mem_n; ++k) {
            int idx = mem_vec[k];
            if (fit[idx] < fit[idx_best])   idx_best  = idx;
            if (fit[idx] > fit[idx_worst])  idx_worst = idx;
        }
        intentar_mejorar_peor(idx_best, idx_worst);
    }
}

// SFL local search sobre la peor rana:
// 1) salto hacia best del memeplex; 2) si no mejora, salto hacia best global;
// 3) si no mejora, re-inicializar aleatoriamente.
void intentar_mejorar_peor(int idx_best, int idx_worst) {
    double cand[R];

    // 1) hacia best del memeplex
    for (int d = 0; d < R; ++d) {
        double diff = (x[idx_best][d] - x[idx_worst][d]);
        double Di = drand48() * diff;
        if (Di >  D_MAX) Di =  D_MAX;
        if (Di < -D_MAX) Di = -D_MAX;
        cand[d] = x[idx_worst][d] + Di;
    }
    clamp_en_rango(cand);
    double f_cand = fitness(cand);
    if (f_cand < fit[idx_worst]) {
        copiar_vec(x[idx_worst], cand);
        fit[idx_worst] = f_cand;
        actualizar_mejor_global_posicion(idx_worst);
        return;
    }

    // 2) hacia best global
    for (int d = 0; d < R; ++d) {
        double diff = (g[d] - x[idx_worst][d]);
        double Di = drand48() * diff;
        if (Di >  D_MAX) Di =  D_MAX;
        if (Di < -D_MAX) Di = -D_MAX;
        cand[d] = x[idx_worst][d] + Di;
    }
    clamp_en_rango(cand);
    f_cand = fitness(cand);
    if (f_cand < fit[idx_worst]) {
        copiar_vec(x[idx_worst], cand);
        fit[idx_worst] = f_cand;
        actualizar_mejor_global_posicion(idx_worst);
        return;
    }

    // 3) aleatoria
    reinit_aleatoria(x[idx_worst]);
    fit[idx_worst] = fitness(x[idx_worst]);
    actualizar_mejor_global_posicion(idx_worst);
}

void actualizar_mejor_global_posicion(int idx) {
    if (fit[idx] < g_fitness) {
        copiar_vec(g, x[idx]);
        g_fitness = fit[idx];
    }
}

void imprimir_progreso(int iter) {
    printf("Iter %3d | Best fitness = %.6f | Best position: [", iter, g_fitness);
    for (int d = 0; d < R; ++d) {
        printf(" %.6f", g[d]);
        if (d < R - 1) printf(",");
    }
    printf(" ]\n");
}

/* ===================== MAIN ===================== */

int main(void) {
    srand48((long)time(NULL));

    inicializar_poblacion();
    ordenar_por_fitness(x, fit, P);

    for (int iter = 0; iter < MAX_ITERS; ++iter) {
        ordenar_por_fitness(x, fit, P);
        if (fit[0] < g_fitness) { copiar_vec(g, x[0]); g_fitness = fit[0]; }

        int mem_idx[M][P];
        int mem_sz[M];
        dividir_en_memeplexes(mem_idx, mem_sz);

        for (int m = 0; m < M; ++m) procesar_memeplex(mem_idx[m], mem_sz[m]);

        if (iter % 5 == 0 || iter == MAX_ITERS - 1) imprimir_progreso(iter);
    }

    printf("\nBest fitness found: %.8f\n", g_fitness);
    printf("Best position: [");
    for (int d = 0; d < R; ++d) {
        printf(" %.8f", g[d]);
        if (d < R - 1) printf(",");
    }
    printf(" ]\n");

    return 0;
}
