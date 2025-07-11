#include "headers.h"
#include "que.h"

int fore_ground = 0;
char COMMAND[] = "";

BackgroundProcess background_processes[1024];
int bg_process_count = 0;
int fg_pid = 0;

int get_newest_pid()
{
    DIR *dir;
    struct dirent *entry;
    int max_pid = -1;

    dir = opendir("/proc");
    if (dir == NULL)
    {
        perror("opendir");
        return -1;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        int pid = atoi(entry->d_name);
        if (pid > max_pid)
        {
            max_pid = pid;
        }
    }

    closedir(dir);
    return max_pid;
}

int kbhit()
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

void check_keypress(int *stop_flag)
{
    if (kbhit())
    {
        char ch = getchar();
        if (ch == 'x' || ch == 'X')
        {
            *stop_flag = 1;
        }
    }
}

void neonate_n(int time_arg)
{
    int stop_flag = 0;
    while (!stop_flag)
    {
        int newest_pid = get_newest_pid();
        if (newest_pid != -1)
        {
            printf("%d\n", newest_pid);
        }
        for (int i = 0; i < 5 * time_arg; i++)
        {
            usleep(200000);
            check_keypress(&stop_flag);
            if (stop_flag)
            {
                break;
            }
        }
    }
    printf("Terminated by user.\n");
}

void handle_sigchld(int sig)
{
    int status;
    int pid;
    int f = 1;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        for (int i = 0; i < bg_process_count; ++i)
        {
            if (background_processes[i].pid == pid)
            {
                if (WIFEXITED(status))
                {
                    printf("%s got terminated\n", background_processes[i].command);
                }
                else if (WIFSIGNALED(status))
                {
                    printf("\033[31m%s got terminated by signal %d\033[0m\n", background_processes[i].command, WTERMSIG(status));
                    f = 0;
                }
                if (f == 1)
                {
                    for (int j = i; j < bg_process_count - 1; ++j)
                    {
                        background_processes[j] = background_processes[j + 1];
                    }
                    bg_process_count--;
                }
                // for (int j = i; j < bg_process_count - 1; ++j)
                // {
                //     background_processes[j] = background_processes[j + 1];
                // }
                // bg_process_count--;
                break;
            }
        }
    }
}

void for_CTRL_C(int signal)
{
    if (fg_pid == 0)
    {
        printf("no fg process running currently\n");
        return;
    }
    if (kill(fg_pid, 0) == 0)
    {
        if (kill(fg_pid, SIGKILL) == 0)
        {
            printf("Sent signal %d to process with pid %d", signal, fg_pid);
            fg_pid = 0;
        }
    }
    else
    {
        printf("no fg process running currently\n");
    }
}

void for_CTRL_Z(int signal)
{
    if (fg_pid == 0)
    {
        printf("No foreground process running currently\n");
        return;
    }
    if (kill(fg_pid, SIGTSTP) == 0)
    {
        printf("Sent signal %d (SIGTSTP) to process with pid %d\n", signal, fg_pid);
        background_processes[bg_process_count].pid = fg_pid;
        snprintf(background_processes[bg_process_count].command, sizeof(background_processes[bg_process_count].command), "%s", COMMAND);
        bg_process_count++;
        fg_pid = 0;
        if (tcsetpgrp(STDIN_FILENO, getpgid(0)) == -1)
        {
            perror("tcsetpgrp failed");
        }
    }
    else
    {
        perror("Failed to send SIGTSTP");
    }
}

void fg_command(int pid)
{
    int found = 0;
    int n_bg = 0;
    for (int i = 0; i < bg_process_count; ++i)
    {
        if (background_processes[i].pid == pid)
        {
            strcpy(COMMAND,background_processes[i].command);
            found = 1;
            for (int j = i; j < bg_process_count - 1; ++j)
            {
                background_processes[j] = background_processes[j + 1];
            }
            bg_process_count--;
            break;
        }
    }
    if (kill(pid, 0) == 0)
    {
        n_bg = 1;
        // printf("it is here");
    }   
    if (n_bg == 1 && found == 1)
    {
        fg_pid = pid;
        int k = kill(pid, SIGCONT);
        if (k < 0)
        {
            perror("Failed to send SIGCONT");
        }
        // if (tcsetpgrp(STDIN_FILENO, getpgid(pid)) == -1)
        // {
        //     perror("Failed to give terminal control to the process");
        //     // printf("here");
        // }
        // signal(SIGTTIN, SIG_IGN);
        // signal(SIGTTOU, SIG_IGN);
        int status;
        waitpid(pid, &status, WUNTRACED);
        // if (WIFSTOPPED(status))
        // {
        //     printf("Process %d stopped\n", pid);
        // }
        // else if (WIFEXITED(status))
        // {
        //     printf("Process %d terminated\n", pid);
        // }
        // if (tcsetpgrp(STDIN_FILENO, getpgrp()) == -1)
        // {
        //     perror("Failed to restore terminal control to shell");
        // }
        // signal(SIGTTIN, SIG_DFL);
        // signal(SIGTTOU, SIG_DFL);

        // fg_pid = 0;
    }

    if (found == 0 && n_bg == 0)
    {
        printf("No such process found\n");
    }
}

void bg_command(int pid)
{
    if (kill(pid, 0) == 0)
    {
        if (kill(pid, SIGCONT) < 0)
        {
            perror("Failed to send SIGCONT");
        }
    }
    else
    {
        printf("process with pid %d doesn't exist", pid);
    }
}

void printinfo(char *curr, char *home)
{
    char username[4096];
    if (getlogin_r(username, sizeof(username)) != 0)
    {
        printf("\033[31merror in getlogin_r\033[0m\n");
    }

    char hostname[4096];
    if (gethostname(hostname, sizeof(hostname)) != 0)
    {
        printf("\033[31merror in hostname\033[0m\n");
    }
    char nor[4096];
    if (getcwd(nor, 4096) == NULL)
    {
        printf("\033[31merror in getcwd\033[0m\n");
    }
    strcpy(curr, nor);
    char display_path[4096];
    path_(curr, home, display_path, sizeof(display_path));

    if (fore_ground == 1)
    {
        printf("<\033[32m%s@\033[0m\033[32m%s\033[0m:\033[34m%s\033[0m \033[37m%s\033[0m> ", username, hostname, display_path, COMMAND);
        fore_ground--;
    }
    else
    {
        printf("<\033[32m%s@\033[0m\033[32m%s\033[0m:\033[34m%s\033[0m> ", username, hostname, display_path);
    }
}

char *get_token_value_from_myshrc(const char *token, const char *myshrc_path)
{
    FILE *file = fopen(myshrc_path, "r");
    if (!file)
    {
        perror("Failed to open .myshrc");
        return NULL;
    }

    char line[4096];
    int token_len = strlen(token);

    while (fgets(line, sizeof(line), file))
    {
        char *token_pos = strstr(line, token);
        if (token_pos && (token_pos == line || *(token_pos - 1) == ' ' || *(token_pos - 1) == '\t'))
        {
            char *equal_pos = strchr(token_pos + token_len, '=');
            if (equal_pos)
            {
                char *value = equal_pos + 1;
                int value_len = strcspn(value, "\n");
                value[value_len] = '\0';
                char *result = malloc(value_len + 1);
                if (result)
                {
                    strcpy(result, value);
                }

                fclose(file);
                return result;
            }
        }
    }

    fclose(file);
    return NULL;
}

int is_directory(const char *path)
{
    struct stat statbuf;
    if (stat(path, &statbuf) == 0)
    {
        return S_ISDIR(statbuf.st_mode);
    }
    return 0;
}

char *get_process_status(pid_t pid)
{
    char status_path[40];
    snprintf(status_path, sizeof(status_path), "/proc/%d/status", pid);

    FILE *status_file = fopen(status_path, "r");
    if (!status_file)
    {
        return "Unknown (Process does not exist or no permission)";
    }

    char line[4096];
    char status_str[16] = "Unknown";
    while (fgets(line, sizeof(line), status_file))
    {
        if (strncmp(line, "State:", 6) == 0)
        {
            sscanf(line, "State:\t%s", status_str);
            break;
        }
    }
    fclose(status_file);

    if (status_str[0] == 'T')
    {
        return "Stopped";
    }
    else if (status_str[0] == 'R' || status_str[0] == 'S')
    {
        return "Running";
    }

    return "Unknown (Unrecognized state)";
}

int cmp_bg_processes(const void *a, const void *b)
{
    BackgroundProcess *procA = (BackgroundProcess *)a;
    BackgroundProcess *procB = (BackgroundProcess *)b;
    return (procA->pid - procB->pid);
}

void execute_command(char *command, int is_background)
{
    int fd_in = -1, fd_out = -1;
    int input_redirect = 0, output_redirect = 0, append_redirect = 0;
    char *input_file = NULL, *output_file = NULL;
    char *cmd_part = strdup(command);
    char *argv[4096];
    int arg_count = 0;
    int in_quotes = 0;      // To track whether we're inside quotes
    char quote_char = '\0'; // To track whether it's a single or double quote
    char *token = (char *)malloc(strlen(cmd_part) + 1);
    int token_index = 0;

    for (int i = 0; cmd_part[i] != '\0'; i++)
    {
        char c = cmd_part[i];

        if ((c == '"' || c == '\'') && !in_quotes)
        {
            in_quotes = 1;
            quote_char = c;
            continue;
        }
        else if (c == quote_char && in_quotes)
        {
            in_quotes = 0;
            quote_char = '\0';
            continue;
        }
        if (in_quotes || (c != ' ' && c != '\t'))
        {
            token[token_index++] = c;
        }
        else
        {
            if (token_index > 0)
            {
                token[token_index] = '\0';
                if (strcmp(token, "<") == 0)
                {
                    input_redirect = 1;
                    input_file = (char *)malloc(256);
                    int file_index = 0;
                    while (cmd_part[++i] == ' ' || cmd_part[i] == '\t')
                        ;

                    while (cmd_part[i] != ' ' && cmd_part[i] != '\t' && cmd_part[i] != '\0')
                    {
                        input_file[file_index++] = cmd_part[i++];
                    }
                    input_file[file_index] = '\0';
                    i--;
                }
                else if (strcmp(token, ">>") == 0)
                {
                    append_redirect = 1;
                    output_file = (char *)malloc(256);
                    int file_index = 0;
                    while (cmd_part[++i] == ' ' || cmd_part[i] == '\t')
                        ;

                    while (cmd_part[i] != ' ' && cmd_part[i] != '\t' && cmd_part[i] != '\0')
                    {
                        output_file[file_index++] = cmd_part[i++];
                    }
                    output_file[file_index] = '\0';
                    i--;
                }
                else if (strcmp(token, ">") == 0)
                {
                    output_redirect = 1;
                    output_file = (char *)malloc(256);
                    int file_index = 0;
                    while (cmd_part[++i] == ' ' || cmd_part[i] == '\t')
                        ;

                    while (cmd_part[i] != ' ' && cmd_part[i] != '\t' && cmd_part[i] != '\0')
                    {
                        output_file[file_index++] = cmd_part[i++];
                    }
                    output_file[file_index] = '\0';
                    i--;
                }
                else
                {
                    argv[arg_count++] = strdup(token);
                }

                token_index = 0;
            }
        }
    }

    if (token_index > 0)
    {
        token[token_index] = '\0';
        argv[arg_count++] = strdup(token);
    }

    argv[arg_count] = NULL;
    free(token);

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        free(cmd_part);
        return;
    }
    else if (pid == 0)
    {
        if (is_background)
        {
            setpgid(0, 0);
            signal(SIGINT, SIG_IGN);
        }

        if (input_redirect)
        {
            fd_in = open(input_file, O_RDONLY);
            if (fd_in < 0)
            {
                perror("open input file");
                exit(EXIT_FAILURE);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }
        if (output_redirect)
        {
            fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out < 0)
            {
                perror("open output file");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }
        if (append_redirect)
        {
            fd_out = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd_out < 0)
            {
                perror("open output file");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }
        if (execvp(argv[0], argv) == -1)
        {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        if (is_background)
        {
            printf("[%d] %s\n", pid, command);
            background_processes[bg_process_count].pid = pid;
            strncpy(background_processes[bg_process_count].command, command, sizeof(background_processes[bg_process_count].command) - 1);
            background_processes[bg_process_count].command[sizeof(background_processes[bg_process_count].command) - 1] = '\0';
            bg_process_count++;
        }
        else
        {
            time_t start_time = time(NULL);
            fg_pid = pid;
            strcpy(COMMAND, command);
            waitpid(pid, NULL, WUNTRACED);
            time_t end_time = time(NULL);
            double elapsed_time = difftime(end_time, start_time);

            // Check if the elapsed time is greater than 2 seconds
            if (elapsed_time > 2.0)
            {
                fore_ground = 1;
            }
        }
        fflush(stdout);
    }
    free(cmd_part);
    free(input_file);
    free(output_file);
}

void activity()
{
    if (bg_process_count == 0)
    {
        printf("No background processes\n");
        return;
    }
    BackgroundProcess sorted_processes[1024];
    for (int i = 0; i < bg_process_count; ++i)
    {
        sorted_processes[i] = background_processes[i];
    }
    qsort(sorted_processes, bg_process_count, sizeof(BackgroundProcess), cmp_bg_processes);
    for (int i = 0; i < bg_process_count; ++i)
    {
        pid_t pid = sorted_processes[i].pid;
        char *command = sorted_processes[i].command;
        char *status = get_process_status(pid);

        printf("[%d] : %s - %s\n", pid, command, status);
    }
}

int main()
{
    signal(SIGCHLD, handle_sigchld);
    char curr_direc[4096];
    char prev_direc[4096];
    char home_direc[4096];

    if (getcwd(home_direc, sizeof(home_direc)) == NULL)
    {
        printf("\033[31mgetcwd\n\033[0m\n");
        return 1;
    }

    strcpy(prev_direc, " ");
    strcpy(curr_direc, home_direc);
    char *file = (char *)malloc(4096 * sizeof(char));
    if (file == NULL)
    {
        printf("\033[31mFailed to allocate memory for file\n\033[0m\n");
        return 1;
    }
    strcpy(file, home_direc);
    strcat(file, "/store.txt");

    queue *que = (queue *)malloc(sizeof(queue));
    if (que == NULL)
    {
        printf("\033[31mFailed to allocate memory for queue\n\033[0m\n");
        free(file);
        return 1;
    }
    init_que(que);

    file_to_que(que, file);
    while (1)
    {
        signal(SIGINT, for_CTRL_C);
        signal(SIGTSTP, for_CTRL_Z);
        printinfo(curr_direc, home_direc);
        char scan[4096];
        // for CTRL-D
        if (fgets(scan, 4096, stdin) == NULL)
        {
            for (int i = 0; i < bg_process_count; i++)
            {
                int p = background_processes[i].pid;
                if (kill(p, 0) == 0)
                {
                    if (kill(p, SIGKILL) == 0)
                    {
                        printf("Sent signal %d to process with pid %d", SIGKILL, p);
                    }
                    else
                    {
                        perror("kill");
                    }
                }
                else
                {
                    printf("process with pid %d doesn't exist", p);
                }
            }
            printf("\nEOF (Ctrl+D) detected.Exiting program.\n");
            return 0;
        }
        else
        {
            scan[strcspn(scan, "\n")] = '\0';
            if (strcmp("stop", scan) == 0)
            {
                break;
            }
            initial_tokenize(scan, curr_direc, prev_direc, home_direc, que, file, 1);
        }
    }

    free(file);
    free(que);
    return 0;
}