#include <stdio.h>
#include <stdlib.h> // Para malloc e EXIT_SUCCESS/EXIT_FAILURE
#include <time.h>   // Para time() em srand
#include <sys/time.h> // Para gettimeofday

// Função para trocar os valores de duas variáveis inteiras
void swap_values(int *first_val_ptr, int *second_val_ptr) {
    int holding_value = *first_val_ptr;
    *first_val_ptr = *second_val_ptr;
    *second_val_ptr = holding_value;
}

// Implementa o algoritmo Odd-Even Transposition Sort de forma serial
void perform_odd_even_sort_serial(int array_to_sort[], int num_elements) {
    int current_sort_phase, element_index;
    for (current_sort_phase = 0; current_sort_phase < num_elements; ++current_sort_phase) {
        // Verifica se a fase atual é par
        if (current_sort_phase % 2 == 0) {
            // Fase Par: compara e troca elementos em posições (i-1, i) onde 'i' é ímpar
            for (element_index = 1; element_index < num_elements; element_index += 2) {
                if (array_to_sort[element_index - 1] > array_to_sort[element_index]) {
                    swap_values(&array_to_sort[element_index - 1], &array_to_sort[element_index]);
                }
            }
        } else {
            // Fase Ímpar: compara e troca elementos em posições (i, i+1) onde 'i' é ímpar
            for (element_index = 1; element_index < num_elements - 1; element_index += 2) {
                if (array_to_sort[element_index] > array_to_sort[element_index + 1]) {
                    swap_values(&array_to_sort[element_index], &array_to_sort[element_index + 1]);
                }
            }
        }
    }
}

// Exibe o conteúdo de um array no console
// Limitado para não imprimir arrays muito grandes completamente
void display_array_segment(int target_array[], int array_length, FILE *output_stream) {
    int display_limit = (array_length > 20) ? 20 : array_length;
    for (int i = 0; i < display_limit; ++i) {
        fprintf(output_stream, "%d ", target_array[i]);
    }
    if (array_length > 20) {
        fprintf(output_stream, "... (apenas os primeiros 20 elementos)\n");
    } else {
        fprintf(output_stream, "\n");
    }
}


// Preenche um array com números inteiros aleatórios
void fill_with_random_numbers(int dest_array[], int total_items, int max_possible_value) {
    srand(time(NULL)); // Inicializa o gerador de números aleatórios com base no tempo atual
    for (int i = 0; i < total_items; ++i) {
        dest_array[i] = rand() % max_possible_value;
    }
}

// Verifica se os elementos de um array estão em ordem não decrescente
int check_if_sorted(int data_array_ptr[], int array_count) {
    for (int idx_check = 0; idx_check < array_count - 1; ++idx_check) {
        if (data_array_ptr[idx_check] > data_array_ptr[idx_check + 1]) {
            return 0; // O array NÃO está ordenado
        }
    }
    return 1; // O array está corretamente ordenado
}

int main(int argc, char *argv[]) {
    // Validação da linha de comando: requer exatamente um argumento (tamanho do array)
    if (argc != 2) {
        fprintf(stderr, "Uso correto: %s <tamanho_do_array>\n", argv[0]);
        return EXIT_FAILURE; // Retorna código de erro
    }

    int array_size = atoi(argv[1]); // Converte o argumento para inteiro
    if (array_size <= 0) {
        fprintf(stderr, "Erro: O tamanho do array deve ser um número inteiro positivo.\n");
        return EXIT_FAILURE;
    }

    // Aloca memória para o array de inteiros
    int *main_array = (int *)malloc(array_size * sizeof(int));
    if (main_array == NULL) {
        fprintf(stderr, "Erro ao alocar memória para o array.\n");
        return EXIT_FAILURE;
    }

    // Variáveis para medição de tempo
    struct timeval start_time_val, end_time_val;
    double elapsed_seconds;
    
    // Preenche o array com valores aleatórios (entre 0 e 999)
    fill_with_random_numbers(main_array, array_size, 1000);
    
    fprintf(stderr, "Array inicial (segmento): ");
    display_array_segment(main_array, array_size, stderr);

    // Inicia a contagem de tempo
    gettimeofday(&start_time_val, NULL);
    
    // Executa o algoritmo de ordenação serial
    perform_odd_even_sort_serial(main_array, array_size);
    
    // Finaliza a contagem de tempo
    gettimeofday(&end_time_val, NULL);

    // Calcula o tempo total decorrido em segundos
    elapsed_seconds = (double)(end_time_val.tv_sec - start_time_val.tv_sec) +
                      (double)(end_time_val.tv_usec - start_time_val.tv_usec) / 1000000.0;

    fprintf(stdout, "Tempo de execução para ordenação serial: %.6f segundos\n", elapsed_seconds);

    fprintf(stderr, "Array final (segmento): ");
    display_array_segment(main_array, array_size, stderr);

    // Verifica e imprime se o array está ordenado
    fprintf(stdout, "Status de ordenação: %s\n", check_if_sorted(main_array, array_size) ? "Ordenado" : "Não Ordenado");
    
    // Libera a memória alocada
    free(main_array);
    main_array = NULL; // Boa prática para evitar 'dangling pointer'

    return EXIT_SUCCESS; // Retorna sucesso
}