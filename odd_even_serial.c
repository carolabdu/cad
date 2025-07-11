#include <stdio.h>
#include <stdlib.h>
#include <time.h> // Para a função clock() e time()

// Função para trocar dois elementos
void swap(int *a, int *b) {
    int temp = *a; [cite_start]// [cite: 14]
    *a = *b; [cite_start]// [cite: 15]
    *b = temp; [cite_start]// [cite: 16]
}

// Implementação serial do Odd-Even Transposition Sort
void odd_even_sort_serial(int arr[], int n) {
    int phase, i; [cite_start]// [cite: 19]
    [cite_start]for (phase = 0; phase < n; phase++) { // [cite: 20]
        [cite_start]if (phase % 2 == 0) { // Fase par [cite: 21]
            // Compara elementos em posições (i-1, i) para i ímpar
            [cite_start]for (i = 1; i < n; i += 2) { // [cite: 23]
                [cite_start]if (arr[i - 1] > arr[i]) { // [cite: 24]
                    swap(&arr[i - 1], &arr[i]); [cite_start]// [cite: 25]
                }
            }
        [cite_start]} else { // Fase ímpar [cite: 28]
            // Compara elementos em posições (i, i+1) para i ímpar
            [cite_start]for (i = 1; i < n - 1; i += 2) { // [cite: 30]
                [cite_start]if (arr[i] > arr[i + 1]) { // [cite: 31]
                    swap(&arr[i], &arr[i + 1]); [cite_start]// [cite: 32]
                }
            }
        }
    }
}

// Função para imprimir o array (parcialmente se for grande)
void print_array(int arr[], int n) {
    [cite_start]for (int i = 0; i < n; i++) { // [cite: 39]
        printf("%d ", arr[i]); // Modificado para adicionar espaço para melhor visualização
    }
    printf("\n"); [cite_start]// [cite: 42]
}

// Função para gerar um array com números aleatórios
void generate_random_array(int arr[], int n, int max_val) {
    srand(time(NULL)); [cite_start]// Inicializa o gerador de números aleatórios com base no tempo [cite: 45]
    [cite_start]for (int i = 0; i < n; i++) { // [cite: 46]
        arr[i] = rand() % max_val; [cite_start]// [cite: 47]
    }
}

// Função para verificar se o array está ordenado
int is_sorted(int arr[], int n) {
    [cite_start]for (int i = 0; i < n - 1; i++) { // [cite: 51]
        [cite_start]if (arr[i] > arr[i + 1]) { // [cite: 52]
            return 0; [cite_start]// Não está ordenado [cite: 53]
        }
    }
    return 1; [cite_start]// Está ordenado [cite: 56]
}

int main(int argc, char *argv[]) {
    // Verifica se o número correto de argumentos foi fornecido
    [cite_start]if (argc != 2) { // [cite: 59]
        printf("Uso: %s <tamanho_array>\n", argv[0]); [cite_start]// [cite: 60]
        return 1; [cite_start]// [cite: 61]
    }

    int n = atoi(argv[1]); [cite_start]// Converte o argumento do tamanho do array para inteiro [cite: 63]
    int *arr = (int *)malloc(n * sizeof(int)); [cite_start]// Aloca memória para o array [cite: 64]

    // Verifica se a alocação de memória foi bem-sucedida
    if (arr == NULL) {
        printf("Erro ao alocar memória.\n");
        return 1;
    }

    // Gerar array aleatório
    generate_random_array(arr, n, 1000); [cite_start]// Gera números até 999 [cite: 66]

    printf("Array original: "); [cite_start]// [cite: 67]
    if (n <= 20) { // Modificado para imprimir até 20 elementos para arrays menores
        print_array(arr, n); [cite_start]// [cite: 68]
    } else {
        print_array(arr, 20); [cite_start]// [cite: 71]
        printf("(exibindo apenas os 20 primeiros elementos)\n"); [cite_start]// [cite: 72]
    }

    // Medição de tempo para a ordenação serial
    clock_t start_time = clock(); // Inicia a contagem do tempo
    odd_even_sort_serial(arr, n); [cite_start]// Chama a função de ordenação serial [cite: 74]
    clock_t end_time = clock();   // Finaliza a contagem do tempo

    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC; // Calcula o tempo decorrido em segundos

    printf("Array ordenado: "); [cite_start]// [cite: 75]
    [cite_start]if (n <= 20) { // [cite: 76]
        print_array(arr, n); [cite_start]// [cite: 79]
    } else {
        print_array(arr, 20); [cite_start]// [cite: 79]
        printf("(exibindo apenas os 20 primeiros elementos)\n"); [cite_start]// [cite: 80]
    }

    // Verifica se o array está ordenado e imprime o resultado
    printf("Array está ordenado: %s\n", is_sorted(arr, n) ? "Sim" : "Não"); [cite_start]// [cite: 82]
    printf("Tempo de execução serial: %.6f segundos\n", elapsed_time); // Imprime o tempo de execução

    free(arr); [cite_start]// Libera a memória alocada [cite: 83]
    return 0; [cite_start]// [cite: 84]
}
