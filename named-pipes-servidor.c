#include "includes.h"

int main()
{
    int fd;

    const char *fifoPath = "/tmp/named_pipe";

    int status = mkfifo(fifoPath, 0666);

    if (status == -1)
    {
        perror("Erro ao criar o FIFO.\n");
        exit(EXIT_FAILURE);
    }

    printf("Servidor de Named Pipes iniciado. Aguardando mensagens...\n");

    while (1)
    {
        fd = open(fifoPath, O_RDONLY);

        if (fd == -1)
        {
            perror("Erro ao abrir o FIFO para leitura.\n");
            exit(EXIT_FAILURE);
        }

        char buffer[100];

        printf("Aguardando mensagem...\n");

        int bytes = read(fd, buffer, sizeof(buffer));

        if (bytes == -1)
        {
            perror("Erro ao ler do FIFO.\n");
            close(fd);
            exit(EXIT_FAILURE);
        }
        else
        {
            buffer[bytes] = '\0';
            printf("Recebido: %s\n", buffer);

            int fdResposta = open(buffer, O_WRONLY);

            if (fdResposta == -1)
            {
                perror("Erro ao abrir o FIFO de resposta para escrita.\n");
                close(fd);
                continue;
            }

            const char *response = "Mensagem recebida com sucesso!";
            int bytesEscritos = write(fdResposta, response, strlen(response));

            if (bytesEscritos == -1)
            {
                perror("Erro ao escrever no FIFO de resposta.\n");
            }
            close(fdResposta);
        }

        sleep(1);

        close(fd);
    }
    return 0;
}