#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

typedef struct
{
    int fd;
    int running;
} ThreadData;

typedef struct
{
    char tipo[20];
    char mensagem[100];
    int tempo;
    char namedPipe[50];
} Mensagem;

typedef struct
{
    pthread_t thread_id;
    char namedPipe[50];
    char mensagem[100];
    int tempo;
} Aviso;

// Considerar que o array de avisos está inicializado com o thread_id = -1 para avisos livres
Aviso avisos[50];

pthread_mutex_t mutexAvisos = PTHREAD_MUTEX_INITIALIZER;

void *funcaoAviso(void *arg)
{
    Aviso *aviso = (Aviso *)arg;

    pthread_mutex_lock(&mutexAvisos);

    int tempo = aviso->tempo;

    pthread_mutex_unlock(&mutexAvisos);

    // Aguardar o tempo especificado
    sleep(tempo);

    pthread_mutex_lock(&mutexAvisos);
    // Abrir o named pipe para escrita
    int fd = open(aviso->namedPipe, O_WRONLY);
    if (fd == -1)
    {
        perror("Erro ao abrir o named pipe do aviso.\n");
        return NULL;
    }

    // Enviar a mensagem
    write(fd, aviso->mensagem, sizeof(aviso->mensagem));

    // Fechar o named pipe
    close(fd);

    // Limpar o aviso
    aviso->thread_id = -1;
    pthread_mutex_unlock(&mutexAvisos);

    return NULL;
}

void *threadNotifica(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    Mensagem mensagem;

    while (data->running)
    {
        read(data->fd, &mensagem, sizeof(mensagem));
        if (strcmp(mensagem.tipo, "aviso") == 0)
        {
            for (int i = 0; i < 50; i++)
            {
                pthread_mutex_lock(&mutexAvisos);
                if (avisos[i].thread_id == -1)
                {
                    strcpy(avisos[i].namedPipe, mensagem.namedPipe);
                    strcpy(avisos[i].mensagem, mensagem.mensagem);
                    avisos[i].tempo = mensagem.tempo;

                    // Criar thread para o aviso
                    pthread_create(&avisos[i].thread_id, NULL, funcaoAviso, (void *)&avisos[i]);

                    pthread_mutex_unlock(&mutexAvisos);

                    break;
                }
                pthread_mutex_unlock(&mutexAvisos);
            }
        }
    }

    return NULL;
}