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
    printf("pid : %d\n", pid);
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    file = fopen(path, "r");
    if (file == NULL)
    {
        perror("failed to open file");
        return;
    }
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
    pid_t fg_pgid = tcgetpgrp(STDOUT_FILENO);
    pid_t proc_pgid = getpgid(pid);

    int is_foreground = (fg_pgid == proc_pgid);
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
    printf("Process Group : %d\n", ppid);
    printf("Virtual memory : %ld kB\n", vmsize);
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
