#include "headers.h"
#include "que.h"

void piping(char *input, char *delimiters, queue *que, char *curr, char *prev, char *home, char *file, int count)
{
    char *commands[1024];
    int cmd_count = 0;
    //printf("%d\n",count);
    // printf("%s",input);
    char *token = strtok(input, "|");

    while (token != NULL)
    {
        trim_whitespace(token);
        if (strlen(token) == 0)
        {
            printf("Invalid use of pipe\n");
            return;
        }
        commands[cmd_count++] = token;
        //printf("%s\n",token);
        token = strtok(NULL, "|");
    }
    // printf("%d\n",cmd_count);
    // printf("%s\n",commands[0]);
    // printf("%s\n",commands[1]);
    int pipefds[2 * (cmd_count - 1)];
    for (int i = 0; i < cmd_count - 1; i++)
    {
        if (pipe(pipefds + i * 2) == -1)
        {
            perror("pipe");
            return;
        }
    }

    int pid;
    for (int i = 0; i < cmd_count; i++)
    {
        pid = fork();
        if (pid == -1)
        {
            perror("fork");
            return;
        }
        else if (pid == 0)
        {
            if (i > 0)
            {
                dup2(pipefds[(i - 1) * 2], STDIN_FILENO);
            }
            if (i < cmd_count - 1)
            {
                dup2(pipefds[i * 2 + 1], STDOUT_FILENO);
            }
            for (int j = 0; j < 2 * (cmd_count - 1); j++)
            {
                close(pipefds[j]);
            }
            // if (count == 1 && i == cmd_count)
            // {
            //     printf("yes\n");
            //     printf("%s",commands[i]);
            //     initial_tokenize(commands[i], curr, prev, home, que, file, 1);
            // }
            // else
            // {
            //     printf("no\n");
            //     initial_tokenize(commands[i], curr, prev, home, que, file, 0);
            // }
            // printf("%s ",commands[i]);
            initial_tokenize(commands[i], curr, prev, home, que, file, 0);
            
            exit(EXIT_FAILURE);
        }    
    }
    for (int i = 0; i < 2 * (cmd_count - 1); i++)
    {
        close(pipefds[i]);
    }
    for (int i = 0; i < cmd_count; i++)
    {
        wait(NULL);
    }
}