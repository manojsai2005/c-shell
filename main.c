#include "headers.h"
#include "que.h"

int fore_ground = 0;
char COMMAND[] = "";
typedef struct
{
    pid_t pid;
    char command[4096];
} BackgroundProcess;

BackgroundProcess background_processes[1024];
int bg_process_count = 0;

void handle_sigchld(int sig)
{
    int status;
    pid_t pid;

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
                }
                for (int j = i; j < bg_process_count - 1; ++j)
                {
                    background_processes[j] = background_processes[j + 1];
                }
                bg_process_count--;
                break;
            }
        }
    }
}

int custom_ceil(double num)
{
    int integer_part = (int)num;
    if (num == (double)integer_part)
    {
        return integer_part;
    }
    else
    {
        return integer_part + 1;
    }
}

int blocks(char *path, char *file)
{
    char temp[4096 * 2];
    struct stat temp_entry;
    strcpy(temp, path);
    strcat(temp, "/");
    strcat(temp, file);
    int block = 0;
    if (stat(temp, &temp_entry) == 0)
    {
        block = custom_ceil((double)temp_entry.st_blocks / 2.0);
    }
    return block;
}

void path_(char *curr, char *home, char *display_path, int size)
{
    if (strncmp(curr, home, strlen(home)) == 0)
    {
        snprintf(display_path, size, "~%s", curr + strlen(home));
    }
    else
    {
        strncpy(display_path, curr, size - 1);
        display_path[size - 1] = '\0';
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
        printf("\033[32m%s@\033[0m\033[32m%s\033[0m:\033[34m%s\033[0m \033[37m%s\033[0m> ", username, hostname, display_path, COMMAND);
        fore_ground--;
    }
    else
    {
        printf("<\033[32m%s@\033[0m\033[32m%s\033[0m:\033[34m%s\033[0m> ", username, hostname, display_path);
    }
}

void initial_tokenize(char *input, char *curr, char *prev, char *home, queue *que, char *file);

void logg(char *token, char *file, queue *que, char **saveptr, char *curr, char *prev, char *home)
{
    if (token == NULL)
    {
        queue_out(que);
    }
    else if (strcmp(token, "purge") == 0)
    {
        clear_que(que);
        que_to_file(que, file);
    }
    else if (strcmp(token, "execute") == 0)
    {
        token = strtok_r(NULL, " ", saveptr);
        int n = atoi(token);
        int r = que->filled - n;
        int want = (r + que->front) % QUEUE_CAPACITY;
        // split_tokens(que->info[want], curr, prev, home, que, file);
        initial_tokenize(que->info[want], curr, prev, home, que, file);
    }
}
void trim_whitespace(char *str)
{
    // Trim leading whitespace
    char *start = str;
    while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r')
        start++;

    // Move the trimmed string to the beginning
    if (str != start)
        memmove(str, start, strlen(start) + 1);

    // Trim trailing whitespace
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
        end--;

    // Null-terminate the string after trimming
    *(end + 1) = '\0';
}

// Function to tokenize input by '&' and pass tokens to split_tokens

void print_with_colour(const char *path, int is_dir, int is_exec)
{
    if (is_dir)
    {
        printf("\033[34m%s\033[0m\n", path); // Blue for directories
    }
    else if (is_exec)
    {
        printf("\033[32m%s\033[0m\n", path); // Green for executables
    }
    else
    {
        printf("%s\n", path); // Default for regular files
    }
}

// Function to check if the path is a directory
int is_directory(const char *path)
{
    struct stat statbuf;
    if (stat(path, &statbuf) == 0)
    {
        return S_ISDIR(statbuf.st_mode);
    }
    return 0;
}

// Function to perform the seek operation

void execute_command(char *command, int is_background)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        printf("\033[31mfork failed\033[0m\n");

        return;
    }
    else if (pid == 0)
    {
        // Child process
        char *argv[] = {"/bin/sh", "-c", command, NULL};
        execvp(argv[0], argv);
        printf("\033[31mexec failed\033[0m\n");
        exit(1);
    }
    else
    {
        // Parent process
        if (is_background)
        {
            printf("[%d] %s\n", pid, command);
            // Store the background process (assuming you have a struct and array for background processes)
            background_processes[bg_process_count].pid = pid;
            strncpy(background_processes[bg_process_count].command, command, sizeof(background_processes[bg_process_count].command) - 1);
            background_processes[bg_process_count].command[sizeof(background_processes[bg_process_count].command) - 1] = '\0';
            bg_process_count++;
        }
        else
        {
            time_t start_time = time(NULL);
            waitpid(pid, NULL, 0);
            time_t end_time = time(NULL);
            double elapsed_time = difftime(end_time, start_time);

            // Check if the elapsed time is greater than 2 seconds
            if (elapsed_time > 2.0)
            {
                fore_ground = 1;
                strcpy(COMMAND, command);
            }
        }
        fflush(stdout);
    }
}

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
        trim_whitespace(token); // Trim leading and trailing whitespace
        split_tokens(token, curr, prev, home, que, file, count);
        count--;
        token = strtok_r(NULL, "&", &saveptr);
    }
}

char *parse_seek_arguments(char *token, char *saveptr, int *only_dirs, int *only_files, int *enable_e_flag, char *curr)
{
    char search[4096];
    char *dir = curr; // Default to current directory
    int search_set = 0;

    while (token != NULL)
    {
        if (strcmp(token, "-d") == 0)
        {
            *only_dirs = 1;
        }
        else if (strcmp(token, "-f") == 0)
        {
            *only_files = 1;
        }
        else if (strcmp(token, "-e") == 0)
        {
            *enable_e_flag = 1;
        }
        else if (search_set == 0)
        {
            strcpy(search, token);
            search_set = 1;
        }
        else
        {
            dir = token;
        }
        token = strtok_r(NULL, " ", &saveptr);
    }

    seek(search, dir, *only_dirs, *only_files, *enable_e_flag);
    return dir;
}

void split_tokens(char *input, char *curr, char *prev, char *home, queue *que, char *file, int count)
{
    char *token;
    char *saveptr;
    int exec = 0;
    int is = 0;
    int c = count;
    char input_copy[4096];
    strncpy(input_copy, input, sizeof(input_copy) - 1);
    input_copy[sizeof(input_copy) - 1] = '\0';

    token = strtok_r(input, " ", &saveptr);
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

            // Update curr based on the result from parse_seek_arguments
            curr = parse_seek_arguments(token, saveptr, &only_dirs, &only_files, &enable_e_flag, curr);
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
            execute_command(input_copy, 1);
        }
        else
        {
            execute_command(input_copy, 0);
        }
    }
}

int search_directory(const char *search_term, const char *dir_path, int dir_flag, int file_flag, int exec_flag);

void seek(const char *search_term, const char *target_directory, int dir_flag, int file_flag, int exec_flag)
{
    // Check for invalid flag combinations
    // printf("%d\n", dir_flag);
    // printf("%d\n", file_flag);
    if (dir_flag && file_flag)
    {
        printf("Invalid flags!\n");
        return;
    }

    // Initialize the directory to search
    if (target_directory == NULL)
    {
        target_directory = ".";
    }

    // Call the recursive search function
    // printf("tt");
    int found = search_directory(search_term, target_directory, dir_flag, file_flag, exec_flag);
    // printf("%d", found);
    if (!found)
    {
        printf("No match found!\n");
    }
}

int search_directory(const char *search_term, const char *dir_path, int dir_flag, int file_flag, int exec_flag)
{
    DIR *dir = opendir(dir_path);
    struct dirent *entry;
    struct stat entry_info;
    int match_count = 0;
    int matching_dir_count = 0;
    char matching_dir[4096] = {0}; // To store the path of the matching directory if exactly one is found

    if (!dir)
    {
        perror("opendir");
        return 0;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir_path, entry->d_name);

        if (stat(fullpath, &entry_info) < 0)
        {
            printf("\033[31mstat failed\033[0m\n");
            continue;
        }

        int is_dir = S_ISDIR(entry_info.st_mode);
        int is_file = S_ISREG(entry_info.st_mode);

        // Apply the filter based on flags
        if ((dir_flag && is_dir) || (file_flag && is_file) || (!dir_flag && !file_flag))
        {
            if (strstr(entry->d_name, search_term) != NULL)
            {
                match_count++;
                if (is_dir)
                {
                    printf("\033[1;34m%s/%s\033[0m\n", dir_path, entry->d_name); // Blue for directories
                    if (exec_flag)
                    {
                        matching_dir_count++;
                        if (matching_dir_count == 1)
                        {
                            strncpy(matching_dir, fullpath, sizeof(matching_dir) - 1);
                        }
                    }
                }
                else
                {
                    printf("\033[1;32m%s/%s\033[0m\n", dir_path, entry->d_name); // Green for files
                    if (exec_flag && match_count == 1)
                    {
                        FILE *file = fopen(fullpath, "r");
                        if (file)
                        {
                            char buffer[4096];
                            while (fgets(buffer, sizeof(buffer), file))
                            {
                                printf("%s", buffer);
                            }
                            fclose(file);
                        }
                        else
                        {
                            printf("\033[31mMissing permissions for task!\033[0m\n");
                        }
                    }
                }
            }
        }

        if (is_dir && !is_file)
        {
            match_count += search_directory(search_term, fullpath, dir_flag, file_flag, exec_flag);
        }
    }

    closedir(dir);

    // Change directory if exactly one directory was matched and the -e flag is set
    if (matching_dir_count == 1 && exec_flag)
    {
        if (chdir(matching_dir) < 0)
        {
            printf("\033[31mchdir\n\033[0m\n");
            printf("Failed to change directory to %s\n", matching_dir);
        }
        else
        {
            // Update the prompt to reflect the new directory
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL)
            {
                printf("Changed directory to: %s\n", cwd);
                // Here you should update your shell prompt logic to reflect the new directory
                // For example:
                // update_prompt(cwd);
            }
            else
            {
                printf("\033[31mgetcwd\n\033[0m\n");
            }
        }
    }

    return match_count;
}

void initial_tokenize(char *input, char *curr, char *prev, char *home, queue *que, char *file)
{
    // Log the entire input line to the queue and file before any tokenization
    int conform = 0;
    if (que->front == -1)
    {
        conform = 1; // Queue is empty, so log the input
    }
    else if (strcmp(input, que->info[que->rear]) != 0)
    {
        conform = 1; // The input is different from the last logged input
    }

    // Use strstr to check if "log" is at the beginning of the input
    if (conform == 1 && (strstr(input, "log") != input))
    {
        if (size(que) == 15)
        {
            deque(que); // Remove the oldest entry if the queue is full
        }
        enque(que, input);      // Add the entire input line to the queue
        que_to_file(que, file); // Write the queue (including the new entry) to the file
    }

    // Now proceed with the tokenization and processing of commands
    char *cmd;
    char *saveptr_cmd;

    // Tokenize the input using ';' as the delimiter
    cmd = strtok_r(input, ";", &saveptr_cmd);
    while (cmd != NULL)
    {
        // Process each command obtained after tokenization
        int count = -1;
        char *cmd_cpy = strdup(cmd); // Create a copy of the command for processing
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

        // Move to the next command
        cmd = strtok_r(NULL, ";", &saveptr_cmd);
    }
}

// Rest of the code remains unchanged

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

    strcpy(prev_direc, home_direc);
    strcpy(curr_direc, home_direc);
    char *file = (char *)malloc(4096 * sizeof(char)); // Allocate memory for file
    if (file == NULL)
    {
        printf("\033[31mFailed to allocate memory for file\n\033[0m\n");
        return 1;
    }
    strcpy(file, home_direc);
    strcat(file, "/store.txt");

    queue *que = (queue *)malloc(sizeof(queue)); // Allocate memory for the queue
    if (que == NULL)
    {
        printf("\033[31mFailed to allocate memory for queue\n\033[0m\n");
        free(file); // Free allocated memory before returning
        return 1;
    }
    init_que(que);

    // Populate the queue from the file before entering the main loop
    file_to_que(que, file);

    while (1)
    {
        printinfo(curr_direc, home_direc);

        char scan[1024];
        if (fgets(scan, sizeof(scan), stdin) != NULL)
        {
            scan[strcspn(scan, "\n")] = '\0'; // Remove newline character

            // Use initial_tokenize to process the input
            if (strcmp("stop", scan) == 0)
            {
                break;
            }
            // printf("ss");
            initial_tokenize(scan, curr_direc, prev_direc, home_direc, que, file);
        }
        else
        {
            printf("\033[31mError reading input.\033[0m\n");
        }
    }

    free(file); // Free allocated memory before exiting
    free(que);  // Free the queue structure memory
    return 0;
}
