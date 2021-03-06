/**
 * Ordenador multithread
 *
 * Rafael Sartori M. Santos, 186154
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// número de threads utilizadas para ordenação
#define NUMERO_THREADS      4
// capacidade inicial do vetor a ser ordenado
#define CAPACIDADE_INICIAL  16


// estrutura para armazenar o vetor
typedef struct {
    int tamanho;
    int capacidade;
    int *vetor;
} vetor_t;

// estrutura a ser passado para o merge sort
typedef struct {
    int *vetor;
    int indice_inicial;
    int indice_final;
} trecho_t;


void inicializar(vetor_t *v) {
    // definimos propriedades
    v->tamanho = 0;
    v->capacidade = CAPACIDADE_INICIAL;
    // alocamos o vetor
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

    // passamos todos os itens do vetor ao novo vetor
    for (int i = 0; i < v->tamanho; i++) {
        novo_vetor[i] = v->vetor[i];
    }

    // substituímos na estrutura o vetor
    free(v->vetor);
    v->vetor = novo_vetor;
}


void inserir(vetor_t *v, int valor) {
    // garantir que há espaço para mais um item
    while ((v->tamanho + 1) >= v->capacidade) {
        expandir(v);
    }

    // inserimos o item no vetor
    v->vetor[v->tamanho] = valor;
    v->tamanho += 1;
}


void liberar(vetor_t *v) {
    // liberamos o vetor
    free(v->vetor);
    // zeramos suas propriedades
    v->tamanho = 0;
    v->capacidade = 0;
}


void quicksort(int *vetor, int inicio, int fim) {
    int i = inicio;
    int j = fim;
    int pivo = vetor[(inicio + fim) / 2];

    while (i <= j) {
        // encontramos posição a esquerda maior que o pivo
        while (vetor[i] < pivo) {
            i++;
        }

        // encontramos posição a direita do pivô que é menor que o pivô
        while (vetor[j] > pivo) {
            j--;
        }

        if (i <= j) {
            int auxiliar = vetor[i];
            vetor[i] = vetor[j];
            vetor[j] = auxiliar;
            i++;
            j--;
        }
    }

    // continuamos recursivamente se o vetor ainda não foi totalmente ordenado
    if (j > inicio) {
        quicksort(vetor, inicio, j);
    }
    if (i < fim) {
        quicksort(vetor, i, fim);
    }
}


void *quicksort_wrapper(void *argumentos) {
    trecho_t *argumento = (trecho_t *) argumentos;
    quicksort(
        argumento->vetor, argumento->indice_inicial, argumento->indice_final
    );
    return NULL;
}


int *merge(int *vetor_A, int tamanho_A, int *vetor_B, int tamanho_B) {
    // tamanho final do vetor
    int tamanho = tamanho_A + tamanho_B;
    // se não há elementros, retornamos NULL
    if (tamanho == 0) {
        return NULL;
    }

    // alocamos vetor final
    int *vetor = malloc(tamanho * sizeof(int));
    if (vetor == NULL) {
        fprintf(stderr, "Falha ao alocar vetor para merge.\n");
        exit(1);
    }

    // posições atuais de cada vetor
    int indice_A = 0;
    int indice_B = 0;
    int indice = 0;

    // criamos o novo vetor ainda ordenado
    while (indice < tamanho) {
        // copiamos condicionalmente o menor ou o que existir
        if (indice_A < tamanho_A && indice_B < tamanho_B &&
            vetor_A[indice_A] < vetor_B[indice_B]) {
            vetor[indice] = vetor_A[indice_A++];
        } else if (indice_B < tamanho_B) {
            vetor[indice] = vetor_B[indice_B++];
        } else if (indice_A < tamanho_A) {
            vetor[indice] = vetor_A[indice_A++];
        }
        indice++;
    }

    return vetor;
}


void imprimir(int *vetor, int tamanho) {
    printf("tamanho = %d, vetor = %p\n", tamanho, vetor);
    for (int i = 0; i < tamanho; i++) {
        printf("%d ", vetor[i]);
    }
    printf("\n");
}


int main() {
    vetor_t v;
    // inicializamos o vetor
    inicializar(&v);

    // colocamos no vetor os inteiros lidos
    {
        int valor;
        while (scanf(" %d%*[, ] ", &valor) != EOF) {
            inserir(&v, valor);
        }
    }

    // abrimos as N threads, executamos ordenação dividida igualmente e a
    // aguardamos
    {
        pthread_t threads[NUMERO_THREADS];
        trecho_t argumentos[NUMERO_THREADS];
        int tamanho_batch = v.tamanho / NUMERO_THREADS;

        for (int i = 0; i < NUMERO_THREADS; i++) {
            trecho_t *args = &(argumentos[i]);

            // iniciamos o vetor de argumentos
            args->vetor = v.vetor;
            // calculamos os índices de acesso do vetor
            args->indice_inicial = i * tamanho_batch;
            if (i == (NUMERO_THREADS - 1)) {
                // se é o final, colocamos o resto
                args->indice_final = v.tamanho - 1;
            } else {
                // se não é o final, dividimos por um número inteiro que é o
                // tamanho do batch
                args->indice_final = args->indice_inicial + tamanho_batch - 1;
            }

            // criamos a thread para ordenar se há itens suficiente
            if ((args->indice_final - args->indice_inicial) > 0) {
                // obs: passamos o endereço args e não o endereço de args pois
                // args é uma variável local
                pthread_create(
                    &(threads[i]), NULL, quicksort_wrapper, args
                );
            }
        }

        int *vetor_acumulador = NULL;
        int tamanho_acumulador = 0;

        // aguardamos a ordenação acabar e fazemos o merge no vetor acumulador
        for (int i = 0; i < NUMERO_THREADS; i++) {
            trecho_t *args = &(argumentos[i]);
            int tamanho = args->indice_final - args->indice_inicial + 1;

            // se há itens suficiente, fazemos o merge
            // obs: entre os índices, o vetor está ordenado
            if (tamanho > 0) {
                // aguardamos a thread acabar de ordenar
                if (tamanho > 1) {
                    pthread_join(threads[i], NULL);
                }

                // fazemos o merge no vetor acumulador
                int *vetor_aux = vetor_acumulador;

                vetor_acumulador = merge(
                    vetor_aux, tamanho_acumulador,
                    v.vetor + args->indice_inicial, tamanho
                );

                // liberamos o antigo vetor acumulador
                if (vetor_aux != NULL) {
                    free(vetor_aux);
                }

                // atualizamos o tamanho do vetor acumulador atual
                tamanho_acumulador += tamanho;
            }
        }

        // conferimos se está tudo certo
        assert(tamanho_acumulador == v.tamanho);

        // substituímos o vetor que temos que ordenar pelo vetor ordenado
        free(v.vetor);
        v.vetor = vetor_acumulador;
    }

    // imprimimos vetor ordenado
    {
        int anterior = v.vetor[0];
        for (int i = 0; i < v.tamanho; i++) {
            printf(i == 0 ? "%d" : " %d", v.vetor[i]);
            if (v.vetor[i] < anterior) {
                fprintf(stderr, "Falha ao ordenar! Indice %d temos %d\n", i, v.vetor[i]);
            }
            anterior = v.vetor[i];
        }
        printf("\n");
    }

    // liberamos memória do vetor
    liberar(&v);

    return 0;
}
