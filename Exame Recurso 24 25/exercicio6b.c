#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

typedef struct
{
    int fd;
    int running;
} ThreadData;

void *threadNotifica(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    char buffer[256];

    while (data->running)
    {
        read(data->fd, buffer, sizeof(buffer));

        // Processa mensagem lida
    }

    return NULL;
}

int main()
{
    ThreadData data;

    // Considerar que o named pipe já foi criado e aberto para leitura no fd da struct

    pthread_t thread_id;

    pthread_create(&thread_id, NULL, threadNotifica, &data);

    char buffer[256];

    do
    {
        printf("Digite uma mensagem para o sistema de avisos (ou 'encerrar' para sair): ");
        fgets(buffer, sizeof(buffer), stdin);

        buffer[strcspn(buffer, "\n")] = 0;

        // Processar comando
    } while (strcmp(buffer, "encerrar") != 0);
}