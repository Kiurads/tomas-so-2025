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
    int fd = open("meu_fifo", O_WRONLY | O_NONBLOCK);

    write(fd, "Mensagem do envia-dados.c\n", 26);
    close(fd);
    return 0;
}