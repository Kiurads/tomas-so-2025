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
#include <errno.h>
#include <time.h>

#define N_PIPES 5

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
    int pid;
    char username[50];
} User;

typedef struct
{
    int pipeFd;
    pthread_t threadId;
    pthread_t timeoutThreadId;
    pthread_cond_t condActive;
    pthread_mutex_t mutexUser;
    User *user;
    int *nErros;
    int *running;

} PipeThread;

void *waitForUserTimeout(void *arg)
{
    PipeThread *pipeThread = (PipeThread *)arg;
    int running = 1;
    int userLoggedIn = 1;

    do
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 30;

        pthread_mutex_lock(&pipeThread->mutexUser);
        int status = pthread_cond_timedwait(&pipeThread->condActive, &pipeThread->mutexUser, &ts);

        if (status < 0)
        {

            pthread_mutex_lock(&pipeThread->mutexUser);

            printf("User %s timed out. Logging out.\n", pipeThread->user->username);

            free(pipeThread->user);
            pipeThread->user = NULL;
            userLoggedIn = 0;

            pthread_mutex_unlock(&pipeThread->mutexUser);
        }

        userLoggedIn = pipeThread->user != NULL;
        pthread_mutex_unlock(&pipeThread->mutexUser);

        pthread_mutex_lock(&mutex);
        running = *(pipeThread->running);
        pthread_mutex_unlock(&mutex);
    } while (running && userLoggedIn);

    return NULL;
}

void *readPipe(void *arg)
{
    PipeThread *pipeThread = (PipeThread *)arg;
    char buffer[1024];
    int bytesRead;

    pthread_mutex_lock(&mutex);

    int running = *(pipeThread->running);

    pthread_mutex_unlock(&mutex);

    do
    {
        bytesRead = read(pipeThread->pipeFd, buffer, sizeof(buffer));
        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0';

            pthread_mutex_lock(&pipeThread->mutexUser);
            pthread_cond_signal(&pipeThread->condActive);
            pthread_mutex_unlock(&pipeThread->mutexUser);

            char *token = strtok(buffer, "_");

            int tipo = atoi(token);

            if (tipo == 1) // Login
            {
                pthread_mutex_lock(&pipeThread->mutexUser);
                if (pipeThread->user != NULL)
                {
                    printf("User %s is already logged in. Ignoring login attempt.\n", pipeThread->user->username);

                    pthread_mutex_unlock(&pipeThread->mutexUser);
                    continue;
                }
                pthread_mutex_unlock(&pipeThread->mutexUser);

                // Process login

                int pid = atoi(strtok(NULL, "_"));
                char *username = strtok(NULL, "_");
                char *resultado = strtok(NULL, "_");

                printf("Login - PID: %d, Username: %s, Resultado: %s\n", pid, username, resultado);

                pthread_mutex_lock(&pipeThread->mutexUser);
                pipeThread->user = malloc(sizeof(User));
                pipeThread->user->pid = pid;
                strcpy(pipeThread->user->username, username);
                pthread_mutex_unlock(&pipeThread->mutexUser);

                pthread_create(&pipeThread->timeoutThreadId, NULL, waitForUserTimeout, pipeThread);
            }
            else if (tipo == 2) // Logout
            {
                // Process logout

                int pid = atoi(strtok(NULL, "_"));
                char *username = strtok(NULL, "_");

                printf("Logout - PID: %d, Username: %s\n", pid, username);

                pthread_mutex_lock(&pipeThread->mutexUser);

                free(pipeThread->user);
                pipeThread->user = NULL;

                pthread_mutex_unlock(&pipeThread->mutexUser);
            }
            else if (tipo == 3) // Resumo
            {
                // Process resumo

                char *username = strtok(NULL, "_");
                int ocorrencias = atoi(strtok(NULL, "_"));
                char *resumo = strtok(NULL, "_");

                printf("Resumo - Username: %s, Ocorrencias: %d, Resumo: %s\n", username, ocorrencias, resumo);
            }
            else if (tipo == 4) // Erro
            {
                // Process erro

                int day = atoi(strtok(NULL, "_"));
                int hour = atoi(strtok(NULL, "_"));
                char *description = strtok(NULL, "_");

                printf("Erro - Day: %d, Hour: %d, Description: %s\n", day, hour, description);

                pthread_mutex_lock(&mutex);
                (*(pipeThread->nErros))++;
                pthread_mutex_unlock(&mutex);
            }
        }

        pthread_mutex_lock(&mutex);

        running = *(pipeThread->running);

        pthread_mutex_unlock(&mutex);
    } while (running);

    return NULL;
}

int main()
{
    PipeThread pipes[N_PIPES];
    int nErros = 0;
    int running = 1;

    for (int i = 0; i < N_PIPES; i++)
    {
        char pipeName[20];
        sprintf(pipeName, "LOGNP%d", i + 1);

        mkfifo(pipeName, 0666);

        pipes[i].pipeFd = open(pipeName, O_RDONLY | O_NONBLOCK);

        pipes[i].nErros = &nErros;
        pipes[i].running = &running;
        pipes[i].user = NULL;

        pthread_mutex_init(&pipes[i].mutexUser, NULL);
        pthread_cond_init(&pipes[i].condActive, NULL);

        pthread_create(&pipes[i].threadId, NULL, readPipe, &pipes[i]);
    }

    do
    {
        char command[100];
        printf("Enter command: ");
        fgets(command, sizeof(command), stdin);

        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "nerros") == 0)
        {
            pthread_mutex_lock(&mutex);
            printf("Number of errors: %d\n", nErros);
            pthread_mutex_unlock(&mutex);
        }
        else if (strcmp(command, "exit") == 0)
        {
            break;
        }
    } while (1);

    pthread_mutex_lock(&mutex);
    running = 0;
    pthread_mutex_unlock(&mutex);

    for (int i = 0; i < N_PIPES; i++)
    {
        pthread_join(pipes[i].threadId, NULL);

        pthread_mutex_lock(&pipes[i].mutexUser);
        if (pipes[i].user != NULL)
        {
            pthread_cond_signal(&pipes[i].condActive);
            pthread_mutex_unlock(&pipes[i].mutexUser);
            pthread_join(pipes[i].timeoutThreadId, NULL);
            pthread_mutex_lock(&pipes[i].mutexUser);
            free(pipes[i].user);
            pipes[i].user = NULL;
        }
        pthread_mutex_unlock(&pipes[i].mutexUser);

        close(pipes[i].pipeFd);

        char pipeName[20];
        sprintf(pipeName, "LOGNP%d", i + 1);
        unlink(pipeName);

        pthread_mutex_destroy(&pipes[i].mutexUser);
        pthread_cond_destroy(&pipes[i].condActive);
    }

    return 0;
}