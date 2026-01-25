#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/wait.h>

int main()
{
    mkfifo("meu_fifo", 0666);

    int fdFifo = open("meu_fifo", O_RDONLY | O_NONBLOCK);
    int fdPipe[2];

    pipe(fdPipe);

    pid_t pid = fork();

    if (pid == 0)
    {
        // Processo filho

        close(fdPipe[0]); // Fecha leitura do pipe
        dup2(fdPipe[1], STDOUT_FILENO);
        close(fdPipe[1]); // Fecha escrita do pipe

        execl("./produz-strings", "./produz-strings", NULL);
    }
    else
    {
        // Processo pai

        close(fdPipe[1]); // Fecha escrita do pipe
    }

    fd_set readfds;

    int maxfd;

    if (fdFifo > fdPipe[0])
    {
        maxfd = fdFifo;
    }
    else
    {
        maxfd = fdPipe[0];
    }

    if (STDIN_FILENO > maxfd)
    {
        maxfd = STDIN_FILENO;
    }

    struct timeval timeout;

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(fdFifo, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        if (pid != -1)
        {
            FD_SET(fdPipe[0], &readfds);
        }

        int ret = select(maxfd + 1, &readfds, NULL, NULL, &timeout);

        if (ret == 0)
        {
            printf("Timeout: Nenhum dado recebido em 2 segundos.\n");
            fflush(stdout);
        }
        else
        {
            if (FD_ISSET(fdFifo, &readfds))
            {
                char buffer[100];
                read(fdFifo, buffer, sizeof(buffer));
                printf("Recebido do FIFO: %s", buffer);
                fflush(stdout);

                close(fdFifo);
                fdFifo = open("meu_fifo", O_RDONLY | O_NONBLOCK);
            }

            if (FD_ISSET(fdPipe[0], &readfds))
            {
                char buffer[100];
                read(fdPipe[0], buffer, sizeof(buffer));
                printf("Recebido do pipe: %s", buffer);
                fflush(stdout);
            }

            if (FD_ISSET(STDIN_FILENO, &readfds))
            {
                char buffer[100];
                fgets(buffer, sizeof(buffer), stdin);
                printf("Recebido do stdin: %s", buffer);
                fflush(stdout);

                if (strcmp(buffer, "mata\n") == 0)
                {
                    kill(pid, SIGTERM);
                    close(fdPipe[0]);
                    waitpid(pid, NULL, 0);
                    pid = -1;
                }

                if (strcmp(buffer, "sair\n") == 0)
                {
                    break;
                }
            }
        }

        timeout.tv_sec = 2;
        timeout.tv_usec = 0;
    }

    if (pid != -1)
    {
        kill(pid, SIGTERM);
        close(fdPipe[0]);
        waitpid(pid, NULL, 0);
    }

    close(fdFifo);
    unlink("meu_fifo");
    return 0;
}