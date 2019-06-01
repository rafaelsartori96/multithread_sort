/**
 * Ordenador multithread
 * 
 * Rafael Sartori M. Santos, 186154
 */
#include <stdio.h>


typedef struct {
    int tamanho;
    int capacidade;
    int *vetor;
} vetor_t;


void inicializar(vetor_t *v) {
    v->tamanho = 0;
    v->capacidade = CAPACIDADE_INICIAL;
    v->vetor = malloc(v->capacidade * sizeof(int));
    if (v->vetor == NULL) {
        fprintf(stderr, "Falha ao alocar vetor inicial! Saindo...\n");
        exit(1);
    }
}


void expandir(vetor_t *v) {
    // realocamos com o dobro de tamanho
    v->capacidade *= 2;
    int *novo_vetor = malloc(v->capacidade * sizeof(int));
    if (novo_vetor == NULL) {
        fprintf(stderr, "Falha ao realocar vetor! Saindo...\n");
        exit(1);
    }

    // passamos todos os itens do vetor
    for (int i = 0; i < v->tamanho; i++) {
        novo_vetor[i] = v->vetor[i];
    }

    // substituímos na estrutura
    free(v->vetor);
    v->vetor = novo_vetor;
}


int main() {
    vetor_t v;
    inicializar(&v);
}
