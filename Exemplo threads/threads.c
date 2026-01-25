#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/wait.h>

int continua = 1;
int mensagensRecebidas = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

#define FIFO_NAME "meu_fifo"

typedef struct
{
    char nomeExecutavel[100];
    int segundosIntervalo;
} DadosThreadPipe;

void *recebeDadosFifo(void *arg)
{
    printf("Thread de recepção de dados do FIFO iniciada.\n");
    fflush(stdout);

    int fdFifo = open(FIFO_NAME, O_RDONLY | O_NONBLOCK);
    char buffer[100];

    while (continua)
    {
        ssize_t bytesRead = read(fdFifo, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            printf("Recebido do FIFO (thread): %s", buffer);
            fflush(stdout);

            pthread_mutex_lock(&mutex);
            mensagensRecebidas++;
            pthread_mutex_unlock(&mutex);
        }
    }

    close(fdFifo);

    printf("Thread de recepção de dados do FIFO finalizada.\n");
    fflush(stdout);
    return NULL;
}

void *recebeDadosPipe(void *arg)
{
    DadosThreadPipe *dados = (DadosThreadPipe *)arg;
    printf("Thread de recepção de dados do pipe iniciada para %s.\n", dados->nomeExecutavel);
    fflush(stdout);

    int fdPipe[2];
    pipe(fdPipe);

    pid_t pid = fork();

    if (pid == 0)
    {
        // Processo filho

        char segundosStr[10];
        sprintf(segundosStr, "%d", dados->segundosIntervalo);
        close(fdPipe[0]); // Fecha leitura do pipe
        dup2(fdPipe[1], STDOUT_FILENO);
        close(fdPipe[1]); // Fecha escrita do pipe

        execl(dados->nomeExecutavel, dados->nomeExecutavel, segundosStr, NULL);
    }
    else
    {
        // Processo pai

        close(fdPipe[1]); // Fecha escrita do pipe

        char buffer[100];
        while (continua)
        {
            ssize_t bytesRead = read(fdPipe[0], buffer, sizeof(buffer) - 1);
            if (bytesRead > 0)
            {
                buffer[bytesRead] = '\0';
                printf("Recebido do pipe (thread): %s", buffer);
                fflush(stdout);

                pthread_mutex_lock(&mutex);
                mensagensRecebidas++;
                pthread_mutex_unlock(&mutex);
            }
        }

        close(fdPipe[0]);
        waitpid(pid, NULL, 0);
    }

    printf("Thread de recepção de dados do pipe finalizada para %s.\n", dados->nomeExecutavel);
    fflush(stdout);
    return NULL;
}

int main()
{
    pthread_t threadFifo;
    pthread_t threadPipe;
    mkfifo(FIFO_NAME, 0666);

    pthread_create(&threadFifo, NULL, recebeDadosFifo, NULL);

    DadosThreadPipe dadosPipe;
    strcpy(dadosPipe.nomeExecutavel, "./produz-strings");
    dadosPipe.segundosIntervalo = 1;

    pthread_create(&threadPipe, NULL, recebeDadosPipe, &dadosPipe);

    while (continua)
    {
        char comando[100];
        fgets(comando, sizeof(comando), stdin);

        pthread_mutex_lock(&mutex);
        mensagensRecebidas++;
        pthread_mutex_unlock(&mutex);

        if (strncmp(comando, "sair", 4) == 0)
        {
            printf("Finalizando o programa.\n");
            fflush(stdout);
            break;
        }
        else if (strncmp(comando, "status", 6) == 0)
        {
            pthread_mutex_lock(&mutex);
            printf("Mensagens recebidas até agora: %d\n", mensagensRecebidas);
            pthread_mutex_unlock(&mutex);
            fflush(stdout);
        }
        else
        {
            printf("Comando desconhecido: %s", comando);
            fflush(stdout);
        }
    }

    continua = 0;

    pthread_join(threadFifo, NULL);
    printf("Thread FIFO finalizada.\n");
    fflush(stdout);

    pthread_join(threadPipe, NULL);
    printf("Thread Pipe finalizada.\n");
    fflush(stdout);

    unlink(FIFO_NAME);
    return 0;
}