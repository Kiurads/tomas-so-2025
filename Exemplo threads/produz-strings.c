#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

void termina(int signum)
{
    exit(0);
}

int main(int argc, char *argv[])
{
    int contador = 0;

    int intervalo = atoi(argv[1]);

    signal(SIGTERM, termina);

    while (1)
    {
        printf("Produzir string %d\n", contador++);
        fflush(stdout);
        sleep(intervalo);
    }
}