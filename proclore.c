#include "headers.h"
#include "que.h"

void process_info(char *token)
{
    int pid;
    if (token == NULL)
    {
        pid = getpid();
    }
    else
    {
        pid = atoi(token);
    }

    char path[4096];
    char line[4096];
    FILE *file;
    char status_str[4096] = "";
    long vmsize = 0;
    pid_t ppid = 0;
    char exe_path[4096] = "";

    // Print PID
    printf("pid : %d\n", pid);

    // Correct the path to /proc/[pid]/status
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    file = fopen(path, "r");
    if (file == NULL)
    {
        perror("failed to open file");
        return;
    }

    // Reading and parsing the /proc/[pid]/status file
    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "State:", 6) == 0)
        {
            sscanf(line, "State:\t%s", status_str);
        }
        else if (strncmp(line, "VmSize:", 7) == 0)
        {
            sscanf(line, "VmSize:\t%ld", &vmsize);
        }
        else if (strncmp(line, "PPid:", 5) == 0)
        {
            sscanf(line, "PPid:\t%d", &ppid);
        }
    }
    fclose(file);

    // Check if the process is in the foreground
    pid_t fg_pgid = tcgetpgrp(STDOUT_FILENO);  // Get the foreground process group of the terminal
    pid_t proc_pgid = getpgid(pid);            // Get the process group ID of the process

    int is_foreground = (fg_pgid == proc_pgid);

    // Print the process status
    if (status_str[0] == 'R')
    {
        if (is_foreground)
        {
            printf("process status : R+\n");
        }
        else
        {
            printf("process status : R\n");
        }
    }
    else if (status_str[0] == 'S')
    {
        if (is_foreground)
        {
            printf("process status : S+\n");
        }
        else
        {
            printf("process status : S\n");
        }
    }
    else if (status_str[0] == 'Z')
    {
        printf("process status : Z\n");
    }
    else
    {
        printf("process status : %s\n", status_str);
    }

    // Print the Process Group
    printf("Process Group : %d\n", ppid);

    // Print the Virtual Memory size
    printf("Virtual memory : %ld kB\n", vmsize);

    // Get executable path
    snprintf(path, sizeof(path), "/proc/%d/exe", pid);
    ssize_t len = readlink(path, exe_path, sizeof(exe_path) - 1);
    if (len != -1)
    {
        exe_path[len] = '\0';
        printf("executable Path : %s\n", exe_path);
    }
    else
    {
        perror("readlink failed");
    }
}
