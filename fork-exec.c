#include "includes.h"

int main()
{
    pid_t pid = fork();

    if (pid == -1)
    {
        perror("Erro no fork.\n");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        printf("%d: Sou o processo filho\n", getpid());

        char *argv[] = {"./teste", NULL};

        int status = execv("./teste", argv);

        if (status == -1)
        {
            perror("Erro no execv.\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf("%d: Sou o processo pai do processo %d\n", getpid(), pid);
    }

    waitpid(pid, NULL, 0);

    printf("%d: Processo filho %d terminou.\n", getpid(), pid);

    return 0;
}