#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // Para strcmp
#include <time.h>
#include <omp.h>

// Troca dois valores inteiros
void swap_values(int *first_val_ptr, int *second_val_ptr) {
    int holding_value = *first_val_ptr;
    *first_val_ptr = *second_val_ptr;
    *second_val_ptr = holding_value;
}

// Exibe até 20 elementos do array
void display_array_segment(int target_array[], int array_length, FILE *output_stream) {
    int display_limit = (array_length > 20) ? 20 : array_length;
    for (int i = 0; i < display_limit; ++i) {
        fprintf(output_stream, "%d ", target_array[i]);
    }
    if (array_length > 20) {
        fprintf(output_stream, "... (exibindo apenas os 20 primeiros elementos)\n");
    } else {
        fprintf(output_stream, "\n");
    }
}

// Preenche array com inteiros aleatórios
void fill_with_random_numbers(int dest_array[], int total_items, int max_possible_value) {
    srand(time(NULL));
    for (int i = 0; i < total_items; ++i) {
        dest_array[i] = rand() % max_possible_value;
    }
}

// Verifica se array está ordenado
int check_if_sorted(int data_array_ptr[], int array_count) {
    for (int idx_check = 0; idx_check < array_count - 1; ++idx_check) {
        if (data_array_ptr[idx_check] > data_array_ptr[idx_check + 1]) {
            return 0;
        }
    }
    return 1;
}

void parallel_odd_even_sort(int array[], int n, int num_threads, const char *policy) {
    omp_set_num_threads(num_threads);

    #pragma omp parallel
    {
        for (int phase = 0; phase < n; ++phase) {
            if (strcmp(policy, "static") == 0) {
                #pragma omp for schedule(static)
                for (int i = (phase % 2); i < n - 1; i += 2) {
                    if (array[i] > array[i + 1]) {
                        swap_values(&array[i], &array[i + 1]);
                    }
                }
            } else if (strcmp(policy, "dynamic") == 0) {
                #pragma omp for schedule(dynamic)
                for (int i = (phase % 2); i < n - 1; i += 2) {
                    if (array[i] > array[i + 1]) {
                        swap_values(&array[i], &array[i + 1]);
                    }
                }
            } else if (strcmp(policy, "guided") == 0) {
                #pragma omp for schedule(guided)
                for (int i = (phase % 2); i < n - 1; i += 2) {
                    if (array[i] > array[i + 1]) {
                        swap_values(&array[i], &array[i + 1]);
                    }
                }
            }

            // Garante que todas as comparações da fase foram feitas
            #pragma omp barrier
            #pragma omp flush(array)
        }
    }
}



int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso correto: %s <tamanho_do_array> <numero_de_threads> <politica: static|dynamic|guided>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int array_size = atoi(argv[1]);
    int thread_count = atoi(argv[2]);
    const char *schedule_policy = argv[3];

    if (array_size <= 0 || thread_count <= 0) {
        fprintf(stderr, "Erro: tamanho do array e número de threads devem ser positivos.\n");
        return EXIT_FAILURE;
    }

    if (!(strcmp(schedule_policy, "static") == 0 || strcmp(schedule_policy, "dynamic") == 0 || strcmp(schedule_policy, "guided") == 0)) {
        fprintf(stderr, "Erro: política de escalonamento deve ser 'static', 'dynamic' ou 'guided'.\n");
        return EXIT_FAILURE;
    }

    int *main_array = (int *)malloc(array_size * sizeof(int));
    if (main_array == NULL) {
        fprintf(stderr, "Erro ao alocar memória.\n");
        return EXIT_FAILURE;
    }

    fill_with_random_numbers(main_array, array_size, 1000);

    fprintf(stderr, "Array original (segmento): ");
    display_array_segment(main_array, array_size, stderr);

    double start_time_stamp = omp_get_wtime();

    parallel_odd_even_sort(main_array, array_size, thread_count, schedule_policy);

    double end_time_stamp = omp_get_wtime();

    fprintf(stdout, "Tempo de execução OpenMP (%d threads, política %s): %.6f segundos\n",
            thread_count, schedule_policy, end_time_stamp - start_time_stamp);

    fprintf(stderr, "Array ordenado (segmento): ");
    display_array_segment(main_array, array_size, stderr);

    fprintf(stdout, "Status de ordenação: %s\n", check_if_sorted(main_array, array_size) ? "Ordenado" : "Não Ordenado");

    free(main_array);
    return EXIT_SUCCESS;
}
