#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int valor = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t disponivel = PTHREAD_COND_INITIALIZER;

void *produtor(void *arg)
{
    while (valor < 100)
    {
        pthread_mutex_lock(&mutex);
        valor++;
        pthread_mutex_unlock(&mutex);
        printf("Valor produzido: %d\n", valor);
        fflush(stdout);
        sleep(1); // Simula tempo de produção
        pthread_cond_signal(&disponivel);
        sleep(1); // Simula tempo de produção
    }
    return NULL;
}

void *consumidor(void *arg)
{
    int valorRecebido = 0;
    while (valorRecebido < 100)
    {
        pthread_cond_wait(&disponivel, &mutex);
        valorRecebido = valor;
        pthread_mutex_unlock(&mutex);

        printf("Valor consumido: %d\n", valorRecebido);
        fflush(stdout);
    }
    return NULL;
}

int main()
{
    pthread_t prodThread, consThread;

    pthread_create(&prodThread, NULL, produtor, NULL);
    pthread_create(&consThread, NULL, consumidor, NULL);

    pthread_join(prodThread, NULL);
    pthread_join(consThread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&disponivel);

    return 0;
}

/*
Dados globais:
int valor = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t disponivel = PTHREAD_COND_INITIALIZER;

Código do produtor:
while (condição) {
    pthread_mutex_lock(&mutex);
    // Produzir valor
    pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&disponivel);
}

Código do consumidor:
while (condição) {
    pthread_cond_wait(&disponivel, &mutex);
    // Consumir valor
    pthread_mutex_unlock(&mutex);
}
*/