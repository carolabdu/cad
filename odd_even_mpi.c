#include <stdio.h>
#include <stdlib.h>
#include <mpi.h> // Necessário para MPI
#include <time.h> // Para srand e time

// Função para trocar dois elementos
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Função para imprimir o array (parcialmente se for grande)
void print_array(int arr[], int n) {
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

// Função para gerar um array com números aleatórios
void generate_random_array(int arr[], int n, int max_val) {
    // Apenas o processo root gera o array completo
    if (MPI_COMM_WORLD) { // Verifica se MPI está inicializado
        srand(time(NULL)); // Inicializa o gerador de números aleatórios com base no tempo
    }
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % max_val;
    }
}

// Função para verificar se o array está ordenado
int is_sorted(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            return 0; // Não está ordenado
        }
    }
    return 1; // Está ordenado
}

int main(int argc, char *argv[]) {
    int rank, size;
    double start_time, end_time, comm_start_time, comm_end_time;
    double total_time_global = 0.0, comm_time_global = 0.0;

    MPI_Init(&argc, &argv); // Inicializa o ambiente MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Obtém o rank (ID) do processo atual
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Obtém o número total de processos

    if (argc != 2) {
        if (rank == 0) {
            printf("Uso: mpirun -np <num_processos> %s <tamanho_array>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    int n_global = atoi(argv[1]); // Tamanho total do array
    int *global_arr = NULL;
    int *local_arr;
    int local_n; // Tamanho do array local para cada processo

    // O processo root aloca o array global
    if (rank == 0) {
        global_arr = (int *)malloc(n_global * sizeof(int));
        if (global_arr == NULL) {
            printf("Erro ao alocar memória para o array global.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        generate_random_array(global_arr, n_global, 1000); // Gera números até 999
    }

    // Calcula o tamanho do array local para cada processo
    // Para simplificar, assume-se que n_global é divisível por size
    // Em uma implementação mais robusta, é necessário lidar com tamanhos desiguais
    if (n_global % size != 0) {
        if (rank == 0) {
            printf("Erro: O tamanho do array (%d) deve ser divisível pelo número de processos (%d).\n", n_global, size);
        }
        if (global_arr != NULL) free(global_arr);
        MPI_Finalize();
        return 1;
    }
    local_n = n_global / size;

    local_arr = (int *)malloc(local_n * sizeof(int));
    if (local_arr == NULL) {
        printf("Processo %d: Erro ao alocar memória para o array local.\n", rank);
        if (global_arr != NULL) free(global_arr);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Distribui o array global para os arrays locais de cada processo
    MPI_Scatter(global_arr, local_n, MPI_INT, local_arr, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    // Início da medição de tempo para a ordenação MPI
    MPI_Barrier(MPI_COMM_WORLD); // Sincroniza todos os processos antes de começar
    start_time = MPI_Wtime();

    // Loop principal do Odd-Even Transposition Sort
    for (int phase = 0; phase < n_global; phase++) {
        // Fase par
        if (phase % 2 == 0) {
            // Compara elementos internos ao processo (posições (i-1, i) para i ímpar)
            for (int i = 1; i < local_n; i += 2) {
                if (local_arr[i - 1] > local_arr[i]) {
                    swap(&local_arr[i - 1], &local_arr[i]);
                }
            }

            // Comunicação com vizinhos (elementos ímpares do processo esquerdo com elementos pares do processo atual)
            // (Compara arr[i-1] e arr[i])
            // Se o processo atual não é o primeiro e tem um elemento par para comparar com o ímpar do vizinho anterior
            if (rank > 0) { // Se não é o primeiro processo
                int partner_rank = rank - 1;
                int recv_val; // Valor a receber do vizinho esquerdo
                int send_val = local_arr[0]; // O primeiro elemento do array local pode precisar ser enviado

                comm_start_time = MPI_Wtime();
                // Envia o primeiro elemento para o vizinho esquerdo e recebe o último elemento do vizinho esquerdo
                MPI_Sendrecv(&send_val, 1, MPI_INT, partner_rank, 0,
                             &recv_val, 1, MPI_INT, partner_rank, 0,
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                comm_end_time = MPI_Wtime();
                comm_time_global += (comm_end_time - comm_start_time);

                if (recv_val > local_arr[0]) {
                    swap(&recv_val, &local_arr[0]); // Troca se necessário
                }
            }
        } else { // Fase ímpar
            // Compara elementos internos ao processo (posições (i, i+1) para i ímpar)
            for (int i = 1; i < local_n - 1; i += 2) {
                if (local_arr[i] > local_arr[i + 1]) {
                    swap(&local_arr[i], &local_arr[i + 1]);
                }
            }

            // Comunicação com vizinhos (elementos pares do processo esquerdo com elementos ímpares do processo atual)
            // (Compara arr[i] e arr[i+1])
            // Se o processo atual não é o último e tem um elemento ímpar para comparar com o par do vizinho posterior
            if (rank < size - 1) { // Se não é o último processo
                int partner_rank = rank + 1;
                int recv_val; // Valor a receber do vizinho direito
                int send_val = local_arr[local_n - 1]; // O último elemento do array local pode precisar ser enviado

                comm_start_time = MPI_Wtime();
                // Envia o último elemento para o vizinho direito e recebe o primeiro elemento do vizinho direito
                MPI_Sendrecv(&send_val, 1, MPI_INT, partner_rank, 0,
                             &recv_val, 1, MPI_INT, partner_rank, 0,
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                comm_end_time = MPI_Wtime();
                comm_time_global += (comm_end_time - comm_start_time);

                if (local_arr[local_n - 1] > recv_val) {
                    swap(&local_arr[local_n - 1], &recv_val); // Troca se necessário
                }
            }
        }
        MPI_Barrier(MPI_COMM_WORLD); // Sincroniza todos os processos após cada fase
    }

    end_time = MPI_Wtime(); // Finaliza a medição de tempo

    // Coleta os arrays locais de volta ao processo root
    MPI_Gather(local_arr, local_n, MPI_INT, global_arr, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    // O processo root processa e imprime os resultados
    if (rank == 0) {
        printf("Array original (primeiros 20): ");
        // O array global_arr no rank 0 agora contém o array ordenado
        // A reimpressão do "original" aqui será o array ordenado.
        // Se quisermos o *original* original, teríamos que fazer uma cópia antes de espalhar.
        // Para este exemplo, vou manter como está, mas é uma observação importante para depuração.
        if (n_global <= 20) {
            print_array(global_arr, n_global);
        } else {
            print_array(global_arr, 20);
            printf("...\n");
        }

        printf("Array ordenado (primeiros 20): ");
        if (n_global <= 20) {
            print_array(global_arr, n_global);
        } else {
            print_array(global_arr, 20);
            printf("...\n");
        }

        printf("Array está ordenado: %s\n", is_sorted(global_arr, n_global) ? "Sim" : "Não");

        // Cálculo do tempo total e overhead de comunicação
        total_time_global = end_time - start_time;

        printf("Tempo de execução MPI (%d processos): %.6f segundos\n", size, total_time_global);
        printf("Tempo de comunicação (acumulado): %.6f segundos\n", comm_time_global);
        if (total_time_global > 0) {
            printf("Overhead de comunicação (relativo): %.2f%%\n", (comm_time_global / total_time_global) * 100);
        }
    }

    // Libera a memória
    free(local_arr);
    if (rank == 0) {
        free(global_arr);
    }

    MPI_Finalize(); // Finaliza o ambiente MPI
    return 0;
}
