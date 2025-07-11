#include "headers.h"
#include "que.h"

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

int compare_strings(const void *a, const void *b)
{
    const char **str_a = (const char **)a;
    const char **str_b = (const char **)b;
    return strcmp(*str_a, *str_b);
}

void get_file_mode(mode_t mode, char *buf)
{
    buf[0] = (S_ISDIR(mode)) ? 'd' : '-';
    buf[1] = (mode & S_IRUSR) ? 'r' : '-';
    buf[2] = (mode & S_IWUSR) ? 'w' : '-';
    buf[3] = (mode & S_IXUSR) ? 'x' : '-';
    buf[4] = (mode & S_IRGRP) ? 'r' : '-';
    buf[5] = (mode & S_IWGRP) ? 'w' : '-';
    buf[6] = (mode & S_IXGRP) ? 'x' : '-';
    buf[7] = (mode & S_IROTH) ? 'r' : '-';
    buf[8] = (mode & S_IWOTH) ? 'w' : '-';
    buf[9] = (mode & S_IXOTH) ? 'x' : '-';
    buf[10] = '\0';
}

void print_colored(const char *name, mode_t mode)
{
    if (S_ISDIR(mode))
    {
        printf("\033[1;34m%s\033[0m\n", name); // Blue for directories
    }
    else
    {
        printf("\033[1;32m%s\033[0m\n", name); // Green for files
    }
}

void print_file_details(struct stat *file_stat, const char *filename)
{
    char mode[11];
    get_file_mode(file_stat->st_mode, mode);

    printf("%s ", mode);
    printf("%ld ", (long)file_stat->st_nlink);

    struct passwd *pw = getpwuid(file_stat->st_uid);
    struct group *gr = getgrgid(file_stat->st_gid);

    if (pw != NULL)
    {
        printf("%s ", pw->pw_name);
    }
    else
    {
        printf("%d ", file_stat->st_uid);
    }

    if (gr != NULL)
    {
        printf("%s ", gr->gr_name);
    }
    else
    {
        printf("%d ", file_stat->st_gid);
    }

    printf("%5lld ", (long long)file_stat->st_size);

    char timebuf[80];
    strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&file_stat->st_mtime));
    printf("%s ", timebuf);
    printf("%s\n", filename);
}

void parse_reveal_options(char *token, char **path, int *show_all, int *long_format, char *curr, char *prev, char *home, char **saveptr)
{
    *show_all = 0;
    *long_format = 0;
    *path = curr;

    while (token != NULL)
    {
        if (token[0] == '-')
        {
            int i = 1;
            while (token[i] != '\0')
            {
                if (token[i] == 'a')
                {
                    *show_all = 1;
                }
                else if (token[i] == 'l')
                {
                    *long_format = 1;
                }
                else
                {
                    fprintf(stderr, "Invalid option: -%c\n", token[i]);
                }
                i++;
            }
        }
        
        else if (token[0] == '~')
        {
            *path = home;
        }
        else if (token[0] == '-' && token[1] == '\0')
        {
            *path = prev;
        }
        else
        {
            *path = token;
        }
        token = strtok_r(NULL, " ", saveptr);
    }
}

void reveal(char *path, int show_all, int long_format)
{
    struct stat file_stat;
    char fullpath[1024];
    char *file_list[1024];
    int file_count = 0;

    if (path == NULL)
    {
        path = ".";
    }

    if (stat(path, &file_stat) < 0)
    {
        printf("\033[31mstat\033[0m\n");
        return;
    }

    if (S_ISDIR(file_stat.st_mode))
    {
        DIR *dir;
        struct dirent *entry;

        if ((dir = opendir(path)) == NULL)
        {
            printf("\033[31mopendir\033[0m\n");
            return;
        }

        // Read directory contents
        while ((entry = readdir(dir)) != NULL)
        {
            if (show_all == 0 && entry->d_name[0] == '.')
            {
                continue;
            }

            strcpy(fullpath, path);
            strcat(fullpath, "/");
            strcat(fullpath, entry->d_name);

            if (stat(fullpath, &file_stat) < 0)
            {
                printf("\033[31mstat\033[0m\n");
                continue;
            }

            file_list[file_count] = strdup(entry->d_name);
            file_count++;
        }
        qsort(file_list, file_count, sizeof(char *), compare_strings);
        int total_blocks = 0;
        for (int i = 0; i < file_count; i++)
        {
            total_blocks += blocks(path, file_list[i]);
        }
        if (long_format)
        {
            printf("total %d\n", total_blocks);
        }
        for (int i = 0; i < file_count; i++)
        {
            strcpy(fullpath, path);
            strcat(fullpath, "/");
            strcat(fullpath, file_list[i]);

            if (stat(fullpath, &file_stat) < 0)
            {
                printf("\033[31mstat\033[0m\n");
                continue;
            }

            if (long_format)
            {
                print_file_details(&file_stat, file_list[i]);
            }
            else
            {
                if (S_ISDIR(file_stat.st_mode))
                {
                    printf("\033[1;34m%s\033[0m\n", file_list[i]); // Blue for directories
                }
                else if (file_stat.st_mode & S_IXUSR)
                {
                    printf("\033[1;32m%s\033[0m\n", file_list[i]); // Green for executables
                }
                else
                {
                    printf("%s\n", file_list[i]); // White for regular files
                }
            }

            free(file_list[i]);
        }

        closedir(dir);
    }
    else
    {
        if (long_format)
        {
            print_file_details(&file_stat, path);
        }
        else
        {
            if (S_ISDIR(file_stat.st_mode))
            {
                printf("\033[1;34m%s\033[0m\n", path); // Blue for directories
            }
            else if (file_stat.st_mode & S_IXUSR)
            {
                printf("\033[1;32m%s\033[0m\n", path); // Green for executables
            }
            else
            {
                printf("%s\n", path); // White for regular files
            }
        }
    }
}
