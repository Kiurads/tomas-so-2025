#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define MAX_IMPRESSOES 20
#define PIPE_NAME "pipe_impressoes"

typedef struct
{
    char nomeFicheiro[50];
    int numCopias;
} PedidoImpressao;

typedef struct
{
    pthread_t thread;
    PedidoImpressao pedido;
    int aImprimir;
    pthread_mutex_t mutex;
} DadosImpressora;

typedef struct
{
    DadosImpressora impressoras[MAX_IMPRESSOES];
    int continua;
    pthread_mutex_t mutex;
} Dados;

void *imprimePedido(void *arg)
{
    DadosImpressora *impressora = (DadosImpressora *)arg;
    int nCopias = 0;
    while (1)
    {
        pthread_mutex_lock(&impressora->mutex);
        if (impressora->aImprimir <= 0 || nCopias >= impressora->pedido.numCopias)
        {
            impressora->aImprimir = 0;
            memset(&impressora->pedido, 0, sizeof(PedidoImpressao));
            pthread_mutex_unlock(&impressora->mutex);
            break;
        }

        nCopias++;

        printf("Copia %d de %d\n", nCopias, impressora->pedido.numCopias);
        fflush(stdout);

        pthread_mutex_unlock(&impressora->mutex);

        sleep(2); // Simula tempo de impressao
    }

    return NULL;
}

void *recebePedidos(void *arg)
{
    Dados *dados = (Dados *)arg;
    int fdPipe = open(PIPE_NAME, O_RDONLY);

    while (1)
    {
        pthread_mutex_lock(&dados->mutex);
        if (!dados->continua)
        {
            pthread_mutex_unlock(&dados->mutex);
            break;
        }
        pthread_mutex_unlock(&dados->mutex);

        PedidoImpressao pedido;
        memset(&pedido, 0, sizeof(PedidoImpressao));
        read(fdPipe, &pedido, sizeof(PedidoImpressao));

        for (int i = 0; i < MAX_IMPRESSOES; i++)
        {
            if (pedido.numCopias <= 0)
            {
                pthread_mutex_lock(&dados->impressoras[i].mutex);
                if (strcmp(dados->impressoras[i].pedido.nomeFicheiro, pedido.nomeFicheiro) == 0)
                {
                    dados->impressoras[i].aImprimir = 0;
                    pthread_mutex_unlock(&dados->impressoras[i].mutex);

                    // Espera pela thread da impressora terminar
                    pthread_join(dados->impressoras[i].thread, NULL);
                    break;
                }
                pthread_mutex_unlock(&dados->impressoras[i].mutex);
            }
            else
            {
                pthread_mutex_lock(&dados->impressoras[i].mutex);
                if (dados->impressoras[i].aImprimir <= 0)
                {
                    strcpy(dados->impressoras[i].pedido.nomeFicheiro, pedido.nomeFicheiro);
                    dados->impressoras[i].pedido.numCopias = pedido.numCopias;
                    dados->impressoras[i].aImprimir = 1;
                    pthread_mutex_unlock(&dados->impressoras[i].mutex);

                    // Cria thread para a impressora
                    pthread_create(&dados->impressoras[i].thread, NULL, imprimePedido, &dados->impressoras[i]);
                    break;
                }
                pthread_mutex_unlock(&dados->impressoras[i].mutex);
            }
        }
    }

    close(fdPipe);
    return NULL;
}

int main()
{
    if (access(PIPE_NAME, F_OK) == 0)
    {
        printf("O gestor ja esta em execucao.\n");
        return 1;
    }

    Dados dados;
    pthread_t threadPedidos;

    for (int i = 0; i < MAX_IMPRESSOES; i++)
    {
        dados.impressoras[i].aImprimir = 0;
        pthread_mutex_init(&dados.impressoras[i].mutex, NULL);
    }

    mkfifo(PIPE_NAME, 0666);
    dados.continua = 1;
    pthread_mutex_init(&dados.mutex, NULL);

    pthread_create(&threadPedidos, NULL, recebePedidos, &dados);
    char buffer[100];
    while (1)
    {
        fgets(buffer, sizeof(buffer), stdin);
        if (strncmp(buffer, "lista", 5) == 0)
        {
            for (int i = 0; i < MAX_IMPRESSOES; i++)
            {
                pthread_mutex_lock(&dados.impressoras[i].mutex);
                if (dados.impressoras[i].aImprimir > 0)
                {
                    printf("Impressora %d: A imprimir %d copias de %s\n", i, dados.impressoras[i].pedido.numCopias, dados.impressoras[i].pedido.nomeFicheiro);
                }
                pthread_mutex_unlock(&dados.impressoras[i].mutex);
            }
        }
        else if (strncmp(buffer, "encerra", 7) == 0)
        {
            break;
        }
    }

    pthread_mutex_lock(&dados.mutex);
    dados.continua = 0;
    pthread_mutex_unlock(&dados.mutex);

    pthread_join(threadPedidos, NULL);
    pthread_mutex_destroy(&dados.mutex);

    for (int i = 0; i < MAX_IMPRESSOES; i++)
    {
        pthread_mutex_lock(&dados.impressoras[i].mutex);
        dados.impressoras[i].aImprimir = 0;
        pthread_mutex_unlock(&dados.impressoras[i].mutex);
    }

    for (int i = 0; i < MAX_IMPRESSOES; i++)
    {
        pthread_join(dados.impressoras[i].thread, NULL);
        pthread_mutex_destroy(&dados.impressoras[i].mutex);
    }

    unlink(PIPE_NAME);
    return 0;
}