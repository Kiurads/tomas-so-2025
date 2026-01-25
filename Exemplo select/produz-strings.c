#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

void termina(int signum)
{
    exit(0);
}

int main()
{
    int contador = 0;

    signal(SIGTERM, termina);

    while (1)
    {
        printf("Produzir string %d\n", contador++);
        fflush(stdout);
        sleep(3);
    }
}