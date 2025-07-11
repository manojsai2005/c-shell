#include "headers.h"
#include "que.h"

void tokenize_by_ampersand(char *input, char *curr, char *prev, char *home, queue *que, char *file, int count)
{
    char *token;
    char *saveptr;
    char input_copy[4096];
    strncpy(input_copy, input, sizeof(input_copy) - 1);
    input_copy[sizeof(input_copy) - 1] = '\0';

    // Tokenize the input by '&'
    token = strtok_r(input_copy, "&", &saveptr);
    while (token != NULL)
    {
        trim_whitespace(token);
        split_tokens(token, curr, prev, home, que, file, count);
        count--;
        token = strtok_r(NULL, "&", &saveptr);
    }
}

void initial_tokenize(char *input, char *curr, char *prev, char *home, queue *que, char *file, int flag)
{
    int conform = 0;
    if (que->front == -1)
    {
        conform = 1;
    }
    else if (strcmp(input, que->info[que->rear]) != 0)
    {
        conform = 1;
    }
    if (conform == 1 && (strstr(input, "log") != input) && flag != 0)
    {
        if (size(que) == 15)
        {
            deque(que);
        }
        enque(que, input);
        que_to_file(que, file);
    }

    char *cmd;
    char *saveptr_cmd;

    cmd = strtok_r(input, ";", &saveptr_cmd);
    while (cmd != NULL)
    {
        int count = -1;
        char *cmd_cpy = strdup(cmd);
        trim_whitespace(cmd_cpy);

        if (cmd_cpy[strlen(cmd_cpy) - 1] == '&')
        {
            count++;
        }
        char *cmd1;
        char *saveptr_cmd1;

        cmd1 = strtok_r(cmd_cpy, "&", &saveptr_cmd1);
        while (cmd1 != NULL)
        {
            count++;
            cmd1 = strtok_r(NULL, "&", &saveptr_cmd1);
        }
        free(cmd_cpy);

        tokenize_by_ampersand(cmd, curr, prev, home, que, file, count);
        cmd = strtok_r(NULL, ";", &saveptr_cmd);
    }
}

void split_tokens(char *input, char *curr, char *prev, char *home, queue *que, char *file, int count)
{
    char input_copy1[4096];
    strcpy(input_copy1, input);
    char *token;
    char *saveptr;
    int exec = 0;
    int is = 0;
    int c = count;
    char input_copy[8193];
    char first_token[4096];
    char remaining_input[4096];
    int i = 0, j = 0;
    while (input[i] != ' ' && input[i] != '\0')
    {
        first_token[j++] = input[i++];
    }
    first_token[j] = '\0';
    if (input[i] == ' ')
    {
        i++; // Skip the space
    }
    strcpy(remaining_input, &input[i]);
    char myshrc_path[4096];
    strcpy(myshrc_path, home);
    strcat(myshrc_path, "/.myshrc");
    char *value_from_myshrc = get_token_value_from_myshrc(first_token, myshrc_path);
    if (value_from_myshrc != NULL)
    {
        strcpy(input_copy, value_from_myshrc);
        strcat(input_copy, " ");
        strcat(input_copy, remaining_input);
        // printf("%s ", input_copy);
        free(value_from_myshrc); // Free memory allocated by get_token_value_from_myshrc
    }
    else
    {
        // No value found, use the original input
        strncpy(input_copy, input, sizeof(input_copy) - 1);
        input_copy[sizeof(input_copy) - 1] = '\0';
    }
    // printf("%s\n", input);
    // char input_copy[4096];
    // strncpy(input_copy, input_copy1, sizeof(input_copy) - 1);
    // input_copy[sizeof(input_copy) - 1] = '\0';
    // printf("%s\n", token);
    char input_copy2[8193];
    strcpy(input_copy2, input_copy);
    if (strstr(input_copy2, "|") != NULL)
    {
        // printf("pipe");
        // char *copy = strdup(token);
        char *delimiters = " ";
        if (count > 0)
        {
            strcat(input_copy2, " &");
        }
        piping(input_copy2, delimiters, que, curr, prev, home, file, count);
        return;
    }
    int sout = dup(STDOUT_FILENO);
    int sin = dup(STDIN_FILENO);
    strcpy(input_copy, check_for_redirection_and_apply(input_copy));
    token = strtok_r(input_copy, " ", &saveptr);
    while (token != NULL)
    {
        trim_whitespace(token);

        if (strcmp(token, "hop") == 0)
        {
            token = strtok_r(NULL, " ", &saveptr);
            while (token != NULL)
            {
                hop(curr, prev, home, token);
                token = strtok_r(NULL, " ", &saveptr);
            }
        }
        else if (strcmp(token, "reveal") == 0)
        {
            int show_all = 0;
            int long_format = 0;
            char *path = NULL;

            token = strtok_r(NULL, " ", &saveptr);
            parse_reveal_options(token, &path, &show_all, &long_format, curr, prev, home, &saveptr);
            reveal(path, show_all, long_format);
        }
        else if (strcmp(token, "log") == 0)
        {
            token = strtok_r(NULL, " ", &saveptr);
            logg(token, file, que, &saveptr, curr, prev, home);
        }
        else if (strcmp(token, "proclore") == 0)
        {
            token = strtok_r(NULL, " ", &saveptr);
            process_info(token);
        }
        else if (strcmp(token, "\n") == 0)
        {
            // Empty command, do nothing
        }
        else if (strcmp(token, "seek") == 0)
        {
            int only_dirs = 0, only_files = 0, enable_e_flag = 0;
            is = 1;
            token = strtok_r(NULL, " ", &saveptr);
            curr = parse_seek_arguments(token, saveptr, &only_dirs, &only_files, &enable_e_flag, curr, prev);
        }
        else if (strcmp(token, "activities") == 0)
        {
            activity();
        }
        else if (strcmp(token, "ping") == 0)
        {
            ping(token, &saveptr);
        }
        else if (strcmp(token, "iMan") == 0)
        {
            token = strtok_r(NULL, " ", &saveptr);
            iMan(token);
        }
        else if (strcmp(token, "fg") == 0)
        {
            token = strtok_r(NULL, " ", &saveptr);
            int pid_ = atoi(token);
            fg_command(pid_);
        }
        else if (strcmp(token, "bg") == 0)
        {
            token = strtok_r(NULL, " ", &saveptr);
            int pid_ = atoi(token);
            bg_command(pid_);
        }
        else if (strcmp(token, "neonate") == 0)
        {
            token = strtok_r(NULL, " ", &saveptr);
            token = strtok_r(NULL, " ", &saveptr);
            int time = atoi(token);
            neonate_n(time);
        }
        else
        {
            exec = 1;
        }

        token = strtok_r(NULL, " ", &saveptr);
    }

    // Execute external commands if needed
    // printf("%d\n", c);
    if (exec == 1 && is == 0)
    {
        if (c > 0)
        {
            execute_command(input_copy2, 1);
        }
        else
        {
            execute_command(input_copy2, 0);
        }
    }
    dup2(sin, STDIN_FILENO);
    dup2(sout, STDOUT_FILENO);
}