#include "headers.h"
#include "que.h"

char *check_for_redirection_and_apply(char *command)
{
    RedirectionInfo result = {0, 0, 0, NULL, NULL};

    char *new_command = malloc(strlen(command) + 1);
    if (new_command == NULL)
        return NULL; 
    new_command[0] = '\0'; 

    int i = 0, cmd_index = 0;
    int length = strlen(command);

    while (i < length)
    {
        if (command[i] == '<')
        { 
            result.input_redirect = 1;
            i++; 
            while (i < length && (command[i] == ' ' || command[i] == '\t'))
                i++;
            int start = i;
            while (i < length && command[i] != ' ' && command[i] != '\t')
                i++;
            result.input_file = strndup(command + start, i - start);
        }
        else if (command[i] == '>' && command[i + 1] == '>')
        { 
            result.append_redirect = 1;
            i += 2;
            while (i < length && (command[i] == ' ' || command[i] == '\t'))
                i++;
            int start = i;
            while (i < length && command[i] != ' ' && command[i] != '\t')
                i++;
            result.output_file = strndup(command + start, i - start);
        }
        else if (command[i] == '>')
        { 
            result.output_redirect = 1;
            i++;
            while (i < length && (command[i] == ' ' || command[i] == '\t'))
                i++;
            int start = i;
            while (i < length && command[i] != ' ' && command[i] != '\t')
                i++;
            result.output_file = strndup(command + start, i - start);
        }
        else
        {
            new_command[cmd_index++] = command[i++];
        }
    }

    new_command[cmd_index] = '\0';
    int fd_in = -1, fd_out = -1;
    if (result.input_redirect)
    {
        fd_in = open(result.input_file, O_RDONLY);
        if (fd_in < 0)
        {
            perror("Error opening input file");
            exit(EXIT_FAILURE);
        }
        if (dup2(fd_in, STDIN_FILENO) < 0)
        { 
            perror("Error redirecting input");
            exit(EXIT_FAILURE);
        }
        close(fd_in);
    }
    if (result.output_redirect)
    {
        fd_out = open(result.output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_out < 0)
        {
            perror("Error opening output file");
            exit(EXIT_FAILURE);
        }
        if (dup2(fd_out, STDOUT_FILENO) < 0)
        { 
            perror("Error redirecting output");
            exit(EXIT_FAILURE);
        }
        close(fd_out);
    }

    if (result.append_redirect)
    {
        fd_out = open(result.output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd_out < 0)
        {
            perror("Error opening output file for append");
            exit(EXIT_FAILURE);
        }
        if (dup2(fd_out, STDOUT_FILENO) < 0)
        {
            perror("Error redirecting output for append");
            exit(EXIT_FAILURE);
        }
        close(fd_out);
    }
    if (result.input_file)
        free(result.input_file);
    if (result.output_file)
        free(result.output_file);
    return new_command;
}