#include "headers.h"
#include "que.h"

void ping(char *token, char **ptr)
{
    token = strtok_r(NULL, " ", ptr);
    int pid = atoi(token);
    token = strtok_r(NULL, " ", ptr);
    int s = atoi(token);
    int signal = s % 32;
    if (kill(pid, 0) == 0)
    {
        if (kill(pid, signal) == 0)
        {
            printf("Sent signal %d to process with pid %d", signal, pid);
        }
        else
        {
            perror("kill");
        }
    }
    else
    {
        printf("process with pid %d doesn't exist", pid);
    }
}