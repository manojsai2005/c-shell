#ifndef FUNCTION_H_
#define FUNCTION_H_

#include "que.h"

void hop(char *curr, char *prev, char *home, char *path);
int custom_ceil(double num);
int blocks(char *path, char *file);
void path_(char *curr, char *home, char *display_path, int size);
void printinfo(char *curr, char *home);
void split_tokens(char *input, char *curr, char *prev, char *home, queue *que, char *file , int count);
void logg(char *token, char *file, queue *que, char **saveptr, char *curr, char *prev, char *home);
int compare_strings(const void *a, const void *b);
void print_file_details(struct stat *file_stat, const char *filename);
void parse_reveal_options(char *token, char **path, int *show_all, int *long_format, char *curr, char *prev,char *home, char **saveptr);
void reveal(char *path, int show_all, int long_format);
void process_info(char *token);
void seek(const char *target_name, const char *path, int only_dirs, int only_files, int exec_flag);

#endif