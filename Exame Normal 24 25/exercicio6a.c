#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define NAMED_PIPE "named_pipe"

int main()
{
    int fd;

    if (access(NAMED_PIPE, F_OK) == 0)
    {
        printf("O programa já está em execução.\n");
        exit(EXIT_FAILURE);
    }

    mkfifo(NAMED_PIPE, 0666);

    fd = open(NAMED_PIPE, O_RDONLY);
}