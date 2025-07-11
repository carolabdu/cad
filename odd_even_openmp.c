#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h> // Necessário para OpenMP

// Função para trocar dois elementos
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Implementação do Odd-Even Transposition Sort com OpenMP
// `schedule_policy`: 0 = static, 1 = dynamic, 2 = guided
void odd_even_sort_openmp(int arr[], int n, int num_threads, int schedule_policy) {
    int phase, i;

    // Define o número de threads para o bloco paralelo externo
    omp_set_num_threads(num_threads);

    #pragma omp parallel // Cada thread entra neste bloco
    {
        for (phase = 0; phase < n; phase++) {
            if (phase % 2 == 0) { // Fase par
                // Paraleliza o loop de comparação para elementos pares
                // A barreira implícita no final do #pragma omp for garante a sincronização entre as fases
                if (schedule_policy == 0) {
                    #pragma omp for schedule(static)
                    for (i = 1; i < n; i += 2) {
                        if (arr[i - 1] > arr[i]) {
                            swap(&arr[i - 1], &arr[i]);
                        }
                    }
                } else if (schedule_policy == 1) {
                    #pragma omp for schedule(dynamic)
                    for (i = 1; i < n; i += 2) {
                        if (arr[i - 1] > arr[i]) {
                            swap(&arr[i - 1], &arr[i]);
                        }
                    }
                } else { // guided
                    #pragma omp for schedule(guided)
                    for (i = 1; i < n; i += 2) {
                        if (arr[i - 1] > arr[i]) {
                            swap(&arr[i - 1], &arr[i]);
                        }
                    }
                }
            } else { // Fase ímpar
                // Paraleliza o loop de comparação para elementos ímpares
                if (schedule_policy == 0) {
                    #pragma omp for schedule(static)
                    for (i = 1; i < n - 1; i += 2) {
                        if (arr[i] > arr[i + 1]) {
                            swap(&arr[i], &arr[i + 1]);
                        }
                    }
                } else if (schedule_policy == 1) {
                    #pragma omp for schedule(dynamic)
                    for (i = 1; i < n - 1; i += 2) {
                        if (arr[i] > arr[i + 1]) {
                            swap(&arr[i], &arr[i + 1]);
                        }
                    }
                } else { // guided
                    #pragma omp for schedule(guided)
                    for (i = 1; i < n - 1; i += 2) {
                        if (arr[i] > arr[i + 1]) {
                            swap(&arr[i], &arr[i + 1]);
                        }
                    }
                }
            }
        }
    }
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
    srand(time(NULL)); // Inicializa o gerador de números aleatórios
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
    // Verifica se o número correto de argumentos foi fornecido
    if (argc < 3 || argc > 4) {
        printf("Uso: %s <tamanho_array> <num_threads> [politica_scheduling]\n", argv[0]);
        printf("  politica_scheduling: 0 (static), 1 (dynamic), 2 (guided) - Padrão: 0 (static)\n");
        return 1;
    }

    int n = atoi(argv[1]); // Tamanho do array
    int num_threads = atoi(argv[2]); // Número de threads
    int schedule_policy = 0; // Padrão: static
    if (argc == 4) {
        schedule_policy = atoi(argv[3]);
        if (schedule_policy < 0 || schedule_policy > 2) {
            printf("Política de scheduling inválida. Usando static (0).\n");
            schedule_policy = 0;
        }
    }

    int *arr = (int *)malloc(n * sizeof(int)); // Aloca memória para o array

    // Verifica se a alocação de memória foi bem-sucedida
    if (arr == NULL) {
        printf("Erro ao alocar memória.\n");
        return 1;
    }

    // Gerar array aleatório
    generate_random_array(arr, n, 1000);

    printf("Array original (primeiros 20): ");
    if (n <= 20) {
        print_array(arr, n);
    } else {
        print_array(arr, 20);
        printf("...\n");
    }

    // Medição de tempo para a ordenação com OpenMP
    double start_time = omp_get_wtime(); // Inicia a contagem do tempo
    odd_even_sort_openmp(arr, n, num_threads, schedule_policy); // Chama a função de ordenação OpenMP
    double end_time = omp_get_wtime();   // Finaliza a contagem do tempo

    double elapsed_time = end_time - start_time; // Calcula o tempo decorrido em segundos

    printf("Array ordenado (primeiros 20): ");
    if (n <= 20) {
        print_array(arr, n);
    } else {
        print_array(arr, 20);
        printf("...\n");
    }

    // Verifica se o array está ordenado e imprime o resultado
    printf("Array está ordenado: %s\n", is_sorted(arr, n) ? "Sim" : "Não");
    printf("Tempo de execução OpenMP (%d threads, %s): %.6f segundos\n",
           num_threads,
           (schedule_policy == 0) ? "static" : ( (schedule_policy == 1) ? "dynamic" : "guided" ),
           elapsed_time);

    free(arr); // Libera a memória alocada
    return 0;
}
