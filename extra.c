#include "headers.h"
#include "que.h"

void trim_whitespace(char *str)
{
    int i = 0, j = 0;
    while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n')
    {
        i++;
    }
    while (str[i] != '\0')
    {
        str[j] = str[i];
        i++;
        j++;
    }
    str[j] = '\0';
    j--;
    while (j >= 0 && (str[j] == ' ' || str[j] == '\t' || str[j] == '\n'))
    {
        j--;
    }
    str[j + 1] = '\0';
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