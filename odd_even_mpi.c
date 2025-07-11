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
