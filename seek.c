#include "headers.h"
#include "que.h"

void seek(const char *search_term, const char *target_directory, int dir_flag, int file_flag, int exec_flag,char *prev,char *curr)
{
    // printf("%d\n", dir_flag);
    // printf("%d\n", file_flag);
    if (dir_flag && file_flag)
    {
        printf("Invalid flags!\n");
        return;
    }

    if (target_directory == NULL)
    {
        target_directory = ".";
    }
    // printf("tt");
    int found = search_directory(search_term, target_directory, dir_flag, file_flag, exec_flag,curr,prev);
    // printf("%d", found);
    if (!found)
    {
        printf("No match found!\n");
    }
}

int search_directory(const char *search_term, const char *dir_path, int dir_flag, int file_flag, int exec_flag,char *curr,char *prev)
{
    DIR *dir = opendir(dir_path);
    struct dirent *entry;
    struct stat entry_info;
    int match_count = 0;
    int matching_dir_count = 0;
    char matching_dir[4096] = {0};

    if (dir == NULL)
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

        if ((dir_flag == 1 && is_dir == 1) || (file_flag == 1 && is_file == 1) || (dir_flag == 0 && file_flag == 0))
        {
            if (strstr(entry->d_name, search_term) != NULL)
            {
                match_count++;
                if (is_dir == 1)
                {
                    printf("\033[1;34m%s/%s\033[0m\n", dir_path, entry->d_name); // Blue for directories
                    if (exec_flag == 1)
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
                    if (exec_flag == 1 && match_count == 1)
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

        if (is_dir == 1 && is_file == 0)
        {
            match_count += search_directory(search_term, fullpath, dir_flag, file_flag, exec_flag,curr,prev);
        }
    }

    closedir(dir);

    if (matching_dir_count == 1 && exec_flag == 1)
    {
        prev = curr;
        if (chdir(matching_dir) < 0)
        {
            printf("\033[31mchdir\n\033[0m\n");
            printf("Failed to change directory to %s\n", matching_dir);
        }
        else
        {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL)
            {
                strcpy(curr,cwd);
                printf("Changed directory to: %s\n", cwd);
            }
            else
            {
                printf("\033[31mgetcwd\n\033[0m\n");
            }
        }
    }

    return match_count;
}

char *parse_seek_arguments(char *token, char *saveptr, int *only_dirs, int *only_files, int *enable_e_flag, char *curr,char *prev)
{
    char search[4096];
    char *dir = curr; 
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

    seek(search, dir, *only_dirs, *only_files, *enable_e_flag,prev,curr);
    return dir;
}