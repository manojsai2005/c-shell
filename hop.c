#include "headers.h"
#include "que.h"

void hop(char *curr, char *prev, char *home, char *path)
{
    char temp[4096];
    char expanded_path[4096];

    // Check if the path starts with ~
    if (path[0] == '~')
    {
        // Expand ~ to the home directory
        snprintf(expanded_path, sizeof(expanded_path), "%s%s", home, path + 1);
        path = expanded_path;
    }

    if (strcmp(path, "-") == 0)
    {
        if (chdir(prev) == 0)
        {
            strcpy(temp, curr);
            if (getcwd(curr, sizeof(temp)) != NULL)
            {
                strcpy(prev, temp);
            }
            else
            {
                perror("getcwd");
            }
        }
        else
        {
            perror("chdir");
        }
    }
    else if (strcmp(path, "~") == 0)
    {
        if (chdir(home) == 0)
        {
            strcpy(prev, curr);
            if (getcwd(curr, sizeof(temp)) == NULL)
            {
                perror("getcwd");
            }
        }
        else
        {
            perror("chdir");
        }
    }
    else if (strcmp(path, ".") == 0)
    {
        // No change needed
    }
    else if (strcmp(path, "..") == 0)
    {
        if (chdir("..") == 0)
        {
            strcpy(prev, curr);
            if (getcwd(curr, sizeof(temp)) == NULL)
            {
                perror("getcwd");
            }
        }
        else
        {
            perror("chdir");
        }
    }
    else
    {
        if (chdir(path) == 0)
        {
            strcpy(prev, curr);
            if (getcwd(curr, sizeof(temp)) == NULL)
            {
                perror("getcwd");
            }
        }
        else
        {
            perror("chdir");
        }
    }

    printf("%s\n", curr);
}
