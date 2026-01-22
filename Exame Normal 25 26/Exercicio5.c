#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/select.h>

char *velocidades[2] = {"80", "120"};

int main()
{
    int tolerancia = atoi(getenv("TOLVELOC"));
    int totalInfracoes = 0;
    int pipeRadar[2][2];
    pid_t pidRadar[2];

    for (int i = 0; i < 2; i++)
    {
        pipe(pipeRadar[i]);
        pidRadar[i] = fork();

        if (pidRadar[i] == 0)
        {
            // Processo filho - Radar

            close(pipeRadar[i][0]);               // Fecha leitura
            dup2(pipeRadar[i][1], STDOUT_FILENO); // Redireciona stdout para o pipe
            close(pipeRadar[i][1]);
            execl("./radar", "./radar", velocidades[i], NULL);
        }
        else
        {
            // Processo pai
            close(pipeRadar[i][1]); // Fecha escrita
        }
    }

    fd_set readfds;

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(pipeRadar[0][0], &readfds);
        FD_SET(pipeRadar[1][0], &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        int maxfd;

        if (pipeRadar[0][0] > pipeRadar[1][0])
        {
            maxfd = pipeRadar[0][0];
        }
        else
        {
            maxfd = pipeRadar[1][0];
        }

        if (STDIN_FILENO > maxfd)
        {
            maxfd = STDIN_FILENO;
        }

        select(maxfd + 1, &readfds, NULL, NULL, NULL);

        for (int i = 0; i < 2; i++)
        {
            if (FD_ISSET(pipeRadar[i][0], &readfds))
            {
                char buffer[100];

                read(pipeRadar[i][0], buffer, sizeof(buffer));

                char *matricula = strtok(buffer, " ");
                char *percentagem = strtok(NULL, " ");

                if (atoi(percentagem) > tolerancia)
                {
                    totalInfracoes++;

                    int pipeMulta[2];

                    pipe(pipeMulta);

                    pid_t pidMulta = fork();

                    if (pidMulta == 0)
                    {
                        // Processo filho - Multa

                        dup2(pipeMulta[0], STDIN_FILENO);  // Redireciona stdin para o pipe
                        close(pipeMulta[0]);               // Fecha leitura
                        dup2(pipeMulta[1], STDOUT_FILENO); // Redireciona stdout para o pipe
                        close(pipeMulta[1]);               // Fecha escrita
                        execl("./multa", "./multa", NULL);
                    }
                    else
                    {
                        write(pipeMulta[1], percentagem, strlen(percentagem) + 1);
                        read(pipeMulta[0], buffer, sizeof(buffer));

                        printf("Radar %d - Matricula: %s - Multa: %s\n", i + 1, matricula, buffer);
                        fflush(stdout);
                        close(pipeMulta[0]);
                        close(pipeMulta[1]);

                        waitpid(pidMulta, NULL, 0);
                    }
                }
            }
        }

        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            char buffer[100];
            scanf("%s", buffer);

            if (strcmp(buffer, "total") == 0)
            {
                printf("Total de infracoes: %d\n", totalInfracoes);
                fflush(stdout);
                continue;
            }

            if (strcmp(buffer, "encerra") == 0)
            {
                break;
            }
        }
    }

    for (int i = 0; i < 2; i++)
    {
        close(pipeRadar[i][0]);
        kill(pidRadar[i], SIGUSR2);
        waitpid(pidRadar[i], NULL, 0);
    }

    return 0;
}