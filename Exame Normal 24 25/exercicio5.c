#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main()
{
    char comando[100];
    char *ficheiro = getenv("LOG_FILE");

    if (ficheiro == NULL)
    {
        fprintf(stderr, "A variável de ambiente LOG_FILE não está definida.\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        printf("Digite um comando (ou 'sair' para encerrar): ");
        fgets(comando, sizeof(comando), stdin);

        // Remover o caractere de nova linha do final da string
        comando[strcspn(comando, "\n")] = 0;

        if (strcmp(comando, "sair") == 0)
        {
            break;
        }

        if (strcmp(comando, "luzes") == 0)
        {
            int fd[2];
            pipe(fd);

            char cor[50];
            char tempo[50];

            printf("Digite a cor das luzes: ");
            fgets(cor, sizeof(cor), stdin);
            cor[strcspn(cor, "\n")] = 0;

            printf("Digite o tempo de duração (em segundos): ");
            fgets(tempo, sizeof(tempo), stdin);
            tempo[strcspn(tempo, "\n")] = 0;

            int pid = fork();

            if (pid == 0)
            {
                // Inicio alinea b
                dup2(fd[1], STDOUT_FILENO);

                close(fd[0]);
                close(fd[1]);
                // Fim alinea b

                char *argv[] = {"./luzes", cor, tempo, NULL};
                execv("./luzes", argv);
                perror("Erro no execv.\n");
                exit(EXIT_FAILURE);
            }
            else
            {
                // Inicio alinea b
                char buffer[100];

                close(fd[1]);

                read(fd[0], buffer, sizeof(buffer));
                printf("Mensagem do processo cortinas: %s\n", buffer);

                saveToFile(ficheiro, buffer, strlen(buffer));
                // Fim alinea b

                waitpid(pid, NULL, 0);
            }
        }
        else if (strcmp(comando, "cortinas") == 0)
        {
            int fd[2];
            pipe(fd);

            int pid = fork();

            if (pid == 0)
            {
                // Inicio alinea b
                dup2(fd[1], STDOUT_FILENO);

                close(fd[0]);
                close(fd[1]);
                // Fim alinea b

                char *argv[] = {"./cortinas", NULL};
                execv("./cortinas", argv);
                perror("Erro no execv.\n");
                exit(EXIT_FAILURE);
            }
            else
            {
                // Inicio alinea b
                char buffer[100];

                close(fd[1]);

                read(fd[0], buffer, sizeof(buffer));
                printf("Mensagem do processo cortinas: %s\n", buffer);

                saveToFile(ficheiro, buffer, strlen(buffer));
                // Fim alinea b

                waitpid(pid, NULL, 0);
            }
        }
        else if (strcmp(comando, "som") == 0)
        {
            int fd[2];
            pipe(fd);

            int pid = fork();

            if (pid == 0)
            {
                // Inicio alinea b
                dup2(fd[1], STDOUT_FILENO);

                close(fd[0]);
                close(fd[1]);
                // Fim alinea b

                char *argv[] = {"./som", NULL};
                execv("./som", argv);
                perror("Erro no execv.\n");
                exit(EXIT_FAILURE);
            }
            else
            {
                // Inicio alinea b
                char buffer[100];

                close(fd[1]);

                read(fd[0], buffer, sizeof(buffer));
                printf("Mensagem do processo som: %s\n", buffer);

                saveToFile(ficheiro, buffer, strlen(buffer));
                // Fim alinea b

                waitpid(pid, NULL, 0);
            }
        }
        else
        {
            printf("Comando desconhecido: %s\n", comando);
        }
    }
}