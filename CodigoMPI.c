#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // Adicionada para corrigir o problema com gethostname
#include <math.h>
#include <time.h>

#define MAXSIZE 65536 // Tamanho do vetor 2^16

// Função para carregar o vetor de um arquivo CSV
void carregar_vetor(char *arquivo, int *vetor, int *tam) {
    FILE *f = fopen(arquivo, "r");
    if (f == NULL) {
        printf("Erro ao abrir o arquivo %s\n", arquivo);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    int i = 0;
    while (fscanf(f, "%d", &vetor[i]) != EOF && i < MAXSIZE) {
        i++;
    }
    *tam = i;
    fclose(f);
}

// Função para realizar a busca sequencial
int busca_sequencial(int *vetor, int tam, int valor) {
    for (int i = 0; i < tam; i++) {
        if (vetor[i] == valor) {
            return i;
        }
    }
    return -1;
}

int main(int argc, char *argv[]) {
    int rank, size;
    int vetor1[MAXSIZE], vetor2[MAXSIZE];
    int tam1, tam2;
    int valor_buscado;
    int resultado = -1;

    // Inicialização do MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        // Solicitar entrada do número a ser buscado
        printf("Digite o valor a ser buscado no vetor: ");
        fflush(stdout);
        scanf("%d", &valor_buscado);
        
        // Mostrar o valor que será buscado
        printf("Valor a ser buscado: %d\n", valor_buscado);
    }

    // Dividir a carga de trabalho (os arquivos) entre os processos
    if (rank == 0) {
        carregar_vetor("vetor1.csv", vetor1, &tam1);
        printf("Processo %d carregou o vetor1.csv com %d elementos.\n", rank, tam1);
    } else if (rank == 1) {
        carregar_vetor("vetor2.csv", vetor2, &tam2);
        printf("Processo %d carregou o vetor2.csv com %d elementos.\n", rank, tam2);
    }

    // Sincronizar os processos para garantir que todos tenham carregado os dados
    MPI_Barrier(MPI_COMM_WORLD);

    // Realizar a busca no vetor correspondente ao processo
    if (rank == 0) {
        resultado = busca_sequencial(vetor1, tam1, valor_buscado);
        if (resultado != -1) {
            printf("Processo %d encontrou o valor %d no vetor1.csv, na posição %d.\n", rank, valor_buscado, resultado);
        }
    } else if (rank == 1) {
        resultado = busca_sequencial(vetor2, tam2, valor_buscado);
        if (resultado != -1) {
            printf("Processo %d encontrou o valor %d no vetor2.csv, na posição %d.\n", rank, valor_buscado, resultado);
        }
    }

    // ------------------------ Alteração solicitada para ajuste da linha
    // Reduzir o resultado (encontrar a posição do valor no vetor) no processo master
    int posicao_final = -1;
    MPI_Reduce(&resultado, &posicao_final, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    // O processo mestre imprime a posição do valor encontrado, ajustada
    if (rank == 0) {
        if (posicao_final != -1) {
            printf("O valor %d foi encontrado na posição %d.\n", valor_buscado, posicao_final + 1); // Aqui é onde fazemos o ajuste de +1 na posição
        } else {
            printf("O valor %d não foi encontrado nos vetores.\n", valor_buscado);
        }
    }

    // Finalizar o MPI
    MPI_Finalize();
    return 0;
}
