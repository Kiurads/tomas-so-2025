#include "includes.h"

int main()
{
    int fdWrite, fdRead;
    const char *fifoPath = "/tmp/named_pipe";
    char responseFifoPath[100];

    snprintf(responseFifoPath, sizeof(responseFifoPath), "/tmp/my_response_pipe_%d", getpid());

    int status = mkfifo(responseFifoPath, 0666);

    if (status == -1)
    {
        perror("Erro ao criar o FIFO de resposta.\n");
        exit(EXIT_FAILURE);
    }

    printf("Cliente de Named Pipes iniciado. Enviando mensagem ao servidor...\n");

    fdWrite = open(fifoPath, O_WRONLY);

    if (fdWrite == -1)
    {
        perror("Erro ao abrir o FIFO para escrita.\n");
        exit(EXIT_FAILURE);
    }

    int bytes = write(fdWrite, responseFifoPath, strlen(responseFifoPath));

    if (bytes == -1)
    {
        perror("Erro ao escrever no FIFO.\n");
        close(fdWrite);
        exit(EXIT_FAILURE);
    }

    printf("Mensagem enviada ao servidor. Aguardando resposta...\n");

    fdRead = open(responseFifoPath, O_RDONLY);

    if (fdRead == -1)
    {
        perror("Erro ao abrir o FIFO de resposta para leitura.\n");
        close(fdWrite);
        exit(EXIT_FAILURE);
    }

    char buffer[100];

    int bytesLidos = read(fdRead, buffer, sizeof(buffer) - 1);

    if (bytesLidos == -1)
    {
        perror("Erro ao ler do FIFO de resposta.\n");
        close(fdRead);
        close(fdWrite);
        exit(EXIT_FAILURE);
    }
    else
    {
        buffer[bytesLidos] = '\0';
        printf("Resposta do servidor: %s\n", buffer);
    }

    close(fdWrite);
    close(fdRead);
    return 0;
}