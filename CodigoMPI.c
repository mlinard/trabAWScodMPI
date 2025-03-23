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
int busca_sequencial(int *vetor, int tam, int valor, int *posicoes) {
    int count = 0;
    for (int i = 0; i < tam; i++) {
        if (vetor[i] == valor) {
            posicoes[count++] = i + 1; // Ajusta para índice baseado em 1
        }
    }
    return count; // Retorna o número de ocorrências encontradas
}

int main(int argc, char *argv[]) {
    int rank, size;
    int vetor1[MAXSIZE], vetor2[MAXSIZE];
    int tam1, tam2;
    int valor_buscado;
    int posicoes1[MAXSIZE], posicoes2[MAXSIZE]; // Para armazenar as posições encontradas
    int num_encontrados1 = 0, num_encontrados2 = 0;

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
        num_encontrados1 = busca_sequencial(vetor1, tam1, valor_buscado, posicoes1);
    } else if (rank == 1) {
        num_encontrados2 = busca_sequencial(vetor2, tam2, valor_buscado, posicoes2);
    }

    // Reduzir as posições encontradas
    int posicoes_finais1[MAXSIZE]; // Para armazenar as posições do vetor 1
    int posicoes_finais2[MAXSIZE]; // Para armazenar as posições do vetor 2
    MPI_Gather(posicoes1, num_encontrados1, MPI_INT, posicoes_finais1, MAXSIZE, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(posicoes2, num_encontrados2, MPI_INT, posicoes_finais2, MAXSIZE, MPI_INT, 0, MPI_COMM_WORLD);

    // O processo mestre imprime as posições dos valores encontrados nos dois vetores
    if (rank == 0) {
        // Resultados para o vetor1
        if (num_encontrados1 > 0) {
            printf("Valor encontrado no vetor1.csv: ");
            for (int i = 0; i < num_encontrados1; i++) {
                printf("%d ", posicoes_finais1[i]);
            }
            printf("\n");
        } else {
            printf("Valor não encontrado no vetor1.csv.\n");
        }

        // Resultados para o vetor2
        if (num_encontrados2 > 0) {
            printf("Valor encontrado no vetor2.csv: ");
            for (int i = 0; i < num_encontrados2; i++) {
                printf("%d ", posicoes_finais2[i]);
            }
            printf("\n");
        } else {
            printf("Valor não encontrado no vetor2.csv.\n");
        }
    }

    // Finalizar o MPI
    MPI_Finalize();
    return 0;
}
