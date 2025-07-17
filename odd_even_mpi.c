#include <stdio.h>
#include <stdlib.h>   // Para malloc, free, qsort, atoi, EXIT_SUCCESS/EXIT_FAILURE
#include <time.h>     // Para time() em srand
#include <mpi.h>      // Para funções MPI
#include <stdint.h>   // Para tipos inteiros (embora não diretamente usados aqui, mantido por ser do original)
#include <string.h>   // Para memset (não usado no original, mas útil se adicionado)
#include <stdbool.h>  // Para tipo bool

// Função para trocar os valores de duas variáveis inteiras
void exchange_values(int *ptr_val_a, int *ptr_val_b) {
    int temporary_holder = *ptr_val_a;
    *ptr_val_a = *ptr_val_b;
    *ptr_val_b = temporary_holder;
}

// Exibe um segmento do array (primeiros 20 elementos ou o total se menor)
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

// Função de comparação para qsort (ordem crescente)
int integer_comparator(const void* val1_ptr, const void* val2_ptr) {
    int arg1 = *(const int*)val1_ptr;
    int arg2 = *(const int*)val2_ptr;
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

// Preenche um array com números inteiros aleatórios
void fill_with_random_elements(int destination_array[], int element_count, int max_value_limit) {
    srand(time(NULL)); // Inicializa o gerador de números aleatórios com base no tempo atual
    for (int i = 0; i < element_count; ++i) {
        destination_array[i] = rand() % max_value_limit;
    }
}

// Verifica se os elementos de um array estão em ordem não decrescente
int check_if_data_is_sorted(int input_data[], int total_elements) {
    for (int i = 0; i < total_elements - 1; ++i) {
        if (input_data[i] > input_data[i + 1]) {
            return 0; // O array NÃO está ordenado
        }
    }
    return 1; // O array está corretamente ordenado
}

// Implementação do Odd-Even Transposition Sort utilizando MPI
double parallel_odd_even_sort_mpi(int local_array_segment[], int global_sorted_array_ptr[],
                                  int total_global_elements, int local_segment_size,
                                  int total_processes, int current_rank) {
    double communication_duration_sum = 0.0; // Tempo acumulado de comunicação
    int boundary_value;
    bool local_swap_occurred;
    int global_swaps_count;

    // Primeiro, ordena o segmento local usando qsort
    qsort(local_array_segment, local_segment_size, sizeof(int), integer_comparator);

    // Loop principal de fases para a ordenação Odd-Even
    for (int sort_iteration = 0; sort_iteration < total_global_elements; ++sort_iteration) {
        local_swap_occurred = false; // Reinicia o flag de troca local para a fase atual

        double comm_start_time = MPI_Wtime(); // Início da medição de tempo de comunicação

        // Fase Par: Processos pares comunicam com processos ímpares à direita
        if (sort_iteration % 2 == 0) {
            if (current_rank % 2 == 0 && current_rank != total_processes - 1) { // Processo par (não o último)
                // Envia o último elemento para o vizinho da direita e recebe do vizinho da direita
                MPI_Sendrecv(&local_array_segment[local_segment_size - 1], 1, MPI_INT, current_rank + 1, 0,
                             &boundary_value, 1, MPI_INT, current_rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                if (local_array_segment[local_segment_size - 1] > boundary_value) {
                    local_array_segment[local_segment_size - 1] = boundary_value;
                    local_swap_occurred = true;
                }
            } else if (current_rank % 2 != 0) { // Processo ímpar
                // Envia o primeiro elemento para o vizinho da esquerda e recebe do vizinho da esquerda
                MPI_Sendrecv(&local_array_segment[0], 1, MPI_INT, current_rank - 1, 0,
                             &boundary_value, 1, MPI_INT, current_rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                if (local_array_segment[0] < boundary_value) {
                    local_array_segment[0] = boundary_value;
                    local_swap_occurred = true;
                }
            }
        } else { // Fase Ímpar: Processos ímpares comunicam com processos pares à direita
            if (current_rank % 2 != 0 && current_rank != total_processes - 1) { // Processo ímpar (não o último)
                // Envia o último elemento para o vizinho da direita e recebe
                MPI_Sendrecv(&local_array_segment[local_segment_size - 1], 1, MPI_INT, current_rank + 1, 0,
                             &boundary_value, 1, MPI_INT, current_rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                if (local_array_segment[local_segment_size - 1] > boundary_value) {
                    local_array_segment[local_segment_size - 1] = boundary_value;
                    local_swap_occurred = true;
                }
            } else if (current_rank % 2 == 0 && current_rank != 0) { // Processo par (não o primeiro)
                // Envia o primeiro elemento para o vizinho da esquerda e recebe
                MPI_Sendrecv(&local_array_segment[0], 1, MPI_INT, current_rank - 1, 0,
                             &boundary_value, 1, MPI_INT, current_rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                if (local_array_segment[0] < boundary_value) {
                    local_array_segment[0] = boundary_value;
                    local_swap_occurred = true;
                }
            }
        }
        double comm_end_time = MPI_Wtime(); // Fim da medição de tempo de comunicação
        communication_duration_sum += (comm_end_time - comm_start_time); // Acumula tempo de comunicação

        // Se uma troca ocorreu, o array local pode ter perdido a ordem interna.
        // Reordena o array local para manter a propriedade de estar localmente ordenado.
        if (local_swap_occurred) {
            // Usa insertion sort para reordenar, pois apenas um elemento mudou
            for (int insert_idx = 1; insert_idx < local_segment_size; ++insert_idx) {
                int current_key = local_array_segment[insert_idx];
                int compare_idx = insert_idx - 1;
                while (compare_idx >= 0 && local_array_segment[compare_idx] > current_key) {
                    local_array_segment[compare_idx + 1] = local_array_segment[compare_idx];
                    compare_idx--;
                }
                local_array_segment[compare_idx + 1] = current_key;
            }
        }

        // Realiza um Allreduce para somar o número de trocas em todos os processos.
        // Isso determina se o algoritmo pode parar mais cedo.
        MPI_Allreduce(&local_swap_occurred, &global_swaps_count, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        if (global_swaps_count == 0) {
            break; // Se nenhuma troca ocorreu em toda a rede, o array está ordenado globalmente
        }
    } // Fim do loop de fases

    // Coleta todos os segmentos locais no processo raiz (rank 0) para formar o array global ordenado
    MPI_Gather(local_array_segment, local_segment_size, MPI_INT,
               global_sorted_array_ptr, local_segment_size, MPI_INT, 0, MPI_COMM_WORLD);

    return communication_duration_sum; // Retorna o tempo total de comunicação para este processo
}


int main(int argc, char *argv[]) {
    // Inicializa o ambiente MPI
    MPI_Init(&argc, &argv);

    // Validação de argumentos: Espera um argumento para o tamanho do array
    if (argc != 2) {
        fprintf(stderr, "Modo de uso: mpiexec -np <num_processos> %s <tamanho_array>\n", argv[0]);
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int overall_array_size = atoi(argv[1]); // Converte o tamanho do array de string para int
    if (overall_array_size <= 0) {
        fprintf(stderr, "Erro: O tamanho do array deve ser um número inteiro positivo.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int process_rank, num_mpi_processes;
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank); // Obtém o rank do processo atual
    MPI_Comm_size(MPI_COMM_WORLD, &num_mpi_processes); // Obtém o número total de processos

    // Calcula o tamanho do segmento de array para cada processo
    // Garante que o array seja divisível igualmente entre os processos
    if (overall_array_size % num_mpi_processes != 0) {
        if (process_rank == 0) {
            fprintf(stderr, "Erro: O tamanho total do array (%d) deve ser divisível pelo número de processos (%d).\n",
                    overall_array_size, num_mpi_processes);
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }
    int local_data_size = overall_array_size / num_mpi_processes;

    int *full_array_master = NULL; // Ponteiro para o array global (apenas no processo raiz)
    int *local_array_segment_ptr = (int *)malloc(local_data_size * sizeof(int)); // Array local para cada processo

    if (local_array_segment_ptr == NULL) {
        fprintf(stderr, "Erro: Falha na alocação de memória para o array local no rank %d.\n", process_rank);
        if (process_rank == 0 && full_array_master != NULL) free(full_array_master);
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    // O processo raiz (rank 0) gera o array completo e o imprime (opcionalmente)
    if (process_rank == 0) {
        full_array_master = (int *)malloc(overall_array_size * sizeof(int));
        if (full_array_master == NULL) {
            fprintf(stderr, "Erro: Falha na alocação de memória para o array global no rank 0.\n");
            free(local_array_segment_ptr);
            MPI_Finalize();
            return EXIT_FAILURE;
        }
        fill_with_random_elements(full_array_master, overall_array_size, 1000); // Preenche com valores aleatórios

        fprintf(stderr, "Array inicial (segmento no Rank 0): ");
        display_array_segment(full_array_master, overall_array_size, stderr);
    }

    // Distribui o array global em segmentos locais para cada processo
    MPI_Scatter(full_array_master, local_data_size, MPI_INT,
                local_array_segment_ptr, local_data_size, MPI_INT, 0, MPI_COMM_WORLD);

    double start_wall_time = MPI_Wtime(); // Início da medição de tempo de execução
    
    // Executa a ordenação Odd-Even Transposition Sort paralela
    double local_comm_time = parallel_odd_even_sort_mpi(local_array_segment_ptr, full_array_master,
                                                         overall_array_size, local_data_size,
                                                         num_mpi_processes, process_rank);
    
    MPI_Barrier(MPI_COMM_WORLD); // Sincroniza todos os processos antes de finalizar a medição de tempo
    double end_wall_time = MPI_Wtime(); // Fim da medição de tempo de execução

    double total_execution_time = end_wall_time - start_wall_time; // Tempo total de execução para este processo

    double summed_comm_time_all_procs;
    double max_total_time_across_procs;

    // Reduz o tempo de comunicação total (soma) e o tempo total de execução (máximo) para o processo raiz
    MPI_Reduce(&local_comm_time, &summed_comm_time_all_procs, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&total_execution_time, &max_total_time_across_procs, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    // O processo raiz imprime os resultados e verifica a ordenação final
    if (process_rank == 0) {
        fprintf(stdout, "\n--- Resultados da Execução MPI ---\n");
        fprintf(stdout, "Tamanho do Array: %d\n", overall_array_size);
        fprintf(stdout, "Número de Processos MPI: %d\n", num_mpi_processes);
        fprintf(stdout, "Tempo de Execução Total (Máximo entre processos): %.6f segundos\n", max_total_time_across_procs);
        fprintf(stdout, "Tempo de Comunicação Total (Soma entre processos): %.6f segundos\n", summed_comm_time_all_procs);
        
        // Calcula e imprime o overhead de comunicação (se o tempo total for diferente de zero para evitar divisão por zero)
        if (max_total_time_across_procs > 0) {
            fprintf(stdout, "Overhead de Comunicação (aproximado): %.2f%%\n", (summed_comm_time_all_procs / max_total_time_across_procs) * 100.0);
        } else {
            fprintf(stdout, "Overhead de Comunicação: N/A (tempo de execução total zero)\n");
        }
        
        fprintf(stdout, "O array final está ordenado: %s\n", check_if_data_is_sorted(full_array_master, overall_array_size) ? "Sim" : "Não");

        fprintf(stderr, "Array Final (segmento no Rank 0): ");
        display_array_segment(full_array_master, overall_array_size, stderr);
    }

    // Libera a memória alocada (apenas o processo raiz libera o array global)
    if (process_rank == 0) {
        free(full_array_master);
        full_array_master = NULL;
    }
    free(local_array_segment_ptr);
    local_array_segment_ptr = NULL;

    // Finaliza o ambiente MPI
    MPI_Finalize();
    return EXIT_SUCCESS;
}