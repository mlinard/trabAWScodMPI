#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // Adicionada para corrigir o problema com gethostname
#include <math.h>
#include <time.h>


#define N 65536  // 2^16 elementos no vetor
#define TAG 0

// Função para gerar um vetor de números únicos
void gerar_vetor_unico(int *vetor, int tamanho) {
    int i, j, num;
    srand(time(NULL));

    for (i = 0; i < tamanho; i++) {
        num = rand() % 1000000;
        for (j = 0; j < i; j++) {
            if (vetor[j] == num) {
                num = rand() % 1000000;
                j = -1;  // Refaz a checagem
            }
        }
        vetor[i] = num;
    }
}

// Função para buscar um elemento no vetor
int buscar_elemento(int *vetor, int tamanho, int valor) {
    for (int i = 0; i < tamanho; i++) {
        if (vetor[i] == valor) {
            return i + 1;  // Retorna a posição baseada em 1
        }
    }
    return -1;  // Retorna -1 se não encontrar o valor
}

int main(int argc, char *argv[]) {
    int myid, numprocs;
    int *vetor;
    int valor_a_buscar = 10;  // Exemplo de valor a ser buscado
    int tamanho_vetor = N;
    int inicio, fim, resultado_local, resultado_global;
    char hostname[30];

    // Inicializa o ambiente MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    gethostname(hostname, 30);

    // O processo master (myid == 0) inicializa o vetor
    if (myid == 0) {
        vetor = (int*) malloc(tamanho_vetor * sizeof(int));
        gerar_vetor_unico(vetor, tamanho_vetor);  // Gera o vetor com valores únicos
    }

    // Distribui o vetor para todos os processos
    MPI_Bcast(vetor, tamanho_vetor, MPI_INT, 0, MPI_COMM_WORLD);

    // Divisão do trabalho: cada processo vai procurar uma parte do vetor
    int chunk_size = tamanho_vetor / numprocs;
    inicio = myid * chunk_size;
    fim = (myid + 1) * chunk_size;

    if (myid == numprocs - 1) {
        fim = tamanho_vetor;  // O último processo pega o restante do vetor
    }

    // Busca o valor no intervalo de cada processo
    resultado_local = buscar_elemento(vetor + inicio, fim - inicio, valor_a_buscar);

    // O processo master coleta os resultados e imprime o primeiro encontrado
    MPI_Reduce(&resultado_local, &resultado_global, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

    if (myid == 0) {
        if (resultado_global != -1) {
            printf("O valor foi encontrado na linha %d.\n", resultado_global);
        } else {
            printf("O valor não foi encontrado no vetor.\n");
        }
    }

    // Libera a memória e finaliza o MPI
    if (myid == 0) {
        free(vetor);
    }

    MPI_Finalize();
    return 0;
}
