#ifndef HEADERS_H_
#define HEADERS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <termios.h>
#include "que.h"
#include "functions.h"
typedef struct
{
    pid_t pid;
    char command[4096];
} BackgroundProcess;
extern BackgroundProcess background_processes[1024];
typedef struct
{
    int input_redirect;
    int output_redirect;
    int append_redirect;
    char *input_file;
    char *output_file;
} RedirectionInfo;

#endif