#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define MAXSIZE 65536 // 2^16

int main(int argc, char **argv) {
    int myid, numprocs;
    int data[MAXSIZE], i, x, low, high, myresult = -1, result = -1;
    FILE *fp;
    const char *file1 = "vetor1.csv";
    const char *file2 = "vetor2.csv";  // Adicionando o segundo arquivo, caso necessário
    int search_value;

    // Inicializa o MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    // Se for o processo mestre, abrir o arquivo e inicializar os dados
    if (myid == 0) {
        fp = fopen(file1, "r");
        if (fp == NULL) {
            printf("Erro ao abrir o arquivo %s\n", file1);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Preenche o vetor com os dados do arquivo
        for (i = 0; i < MAXSIZE; i++) {
            fscanf(fp, "%d\n", &data[i]);
        }
        fclose(fp);

        // Solicita ao usuário o valor para busca
        printf("Digite o valor a ser buscado no vetor: ");
        scanf("%d", &search_value);
    }

    // Difunde os dados para todos os processos
    MPI_Bcast(data, MAXSIZE, MPI_INT, 0, MPI_COMM_WORLD);
    // Difunde o valor a ser buscado
    MPI_Bcast(&search_value, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Divide os dados entre os processos
    x = MAXSIZE / numprocs;
    low = myid * x;
    high = (myid == numprocs - 1) ? MAXSIZE : (low + x);

    // Realiza a busca no intervalo atribuído
    for (i = low; i < high; i++) {
        if (data[i] == search_value) {
            myresult = i;  // Armazena a posição do elemento encontrado
            break;
        }
    }

    // Imprime o resultado da busca
    char hostname[30];
    gethostname(hostname, 30);
    printf("Processo %d (%s) encontrou o valor em %d\n", myid, hostname, myresult);

    // Recolhe os resultados dos outros processos
    MPI_Reduce(&myresult, &result, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    // O processo mestre imprime o resultado final
    if (myid == 0) {
        if (result != -1) {
            printf("O valor foi encontrado na posição %d\n", result);
        } else {
            printf("O valor não foi encontrado no vetor\n");
        }
    }

    // Finaliza o MPI
    MPI_Finalize();

    return 0;
}
