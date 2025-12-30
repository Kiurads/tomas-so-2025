#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>

#define MAX_APPS 50

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pid_t pids[MAX_APPS];

typedef struct
{
    pthread_t thread;
    pid_t pid;
    pid_t *pidApps;
} DadosAplicacao;

void recebeAlarme(int signum)
{
    pthread_mutex_lock(&mutex);

    for (int i = 0; i < MAX_APPS; i++)
    {
        if (pids[i] > 0)
        {
            kill(pids[i], SIGTERM);
        }
    }

    pthread_mutex_unlock(&mutex);

    printf("0");
}

void *leituraApp1(void *arg)
{
    int fd = *(int *)arg;
    int bytesRead;
    char pidBuffer[100];

    while ((bytesRead = read(fd, pidBuffer, sizeof(pidBuffer))) > 0)
    {
        printf("App1 output [%d bytes]: %s\n", bytesRead, pidBuffer);
    }

    return NULL;
}

void *aguardaAplicacao(void *arg)
{
    DadosAplicacao *dados = (DadosAplicacao *)arg;
    int status;
    int seconds = 0;

    while (1)
    {
        pid_t result = waitpid(dados->pid, &status, WNOHANG);

        if (result == 0)
        {
            sleep(1);
            seconds++;
            continue;
        }
        else if (result == -1)
        {
            perror("waitpid");
            return NULL;
        }
        else
        {
            break;
        }
    }

    if (WIFSIGNALED(status))
    {
        return NULL;
    }

    printf("Aplicacao com PID %d terminou em %d segundos.\n", dados->pid, seconds);

    pthread_mutex_lock(&mutex);

    for (int i = 0; i < MAX_APPS; i++)
    {
        if (dados->pidApps[i] != dados->pid)
        {
            kill(dados->pidApps[i], SIGTERM);
        }
    }

    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char *argv[])
{
    // Alinea a)
    DadosAplicacao dados[MAX_APPS];
    int numApps = argc - 1;

    // Alinea c)
    pthread_t threadLeitura;
    int fd[2];

    pipe(fd);

    for (int i = 0; i < numApps; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // Alinea c)
            if (i == 0)
            {
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
            }

            execlp(argv[i + 1], argv[i + 1], NULL);
        }
        else
        {
            // Alinea c)
            if (i == 0)
            {
                close(fd[1]);
                pthread_create(&threadLeitura, NULL, (void *)leituraApp1, &fd[0]);
                close(fd[0]);
            }

            dados[i].pid = pid;

            dados[i].pidApps = pids;

            pthread_create(&dados[i].thread, NULL, aguardaAplicacao, &dados[i]);
        }
    }

    // Alinea b)
    int tempoInt = atoi(getenv("TEMPO"));

    signal(SIGALRM, recebeAlarme);

    alarm(tempoInt);

    return EXIT_SUCCESS;
}