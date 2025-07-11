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
void seek(const char *target_name, const char *path, int only_dirs, int only_files, int exec_flag,char *prev,char *curr);
int search_directory(const char *search_term, const char *dir_path, int dir_flag, int file_flag, int exec_flag,char *curr,char *prev);
void initial_tokenize(char *input, char *curr, char *prev, char *home, queue *que, char *file,int flag);
char *parse_seek_arguments(char *token, char *saveptr, int *only_dirs, int *only_files, int *enable_e_flag, char *curr,char *prev);
void execute_command(char *command, int is_background);
void piping(char *input, char *delimiters, queue *que, char *curr, char *prev, char *home, char *file, int count);
void trim_whitespace(char *str);
void print_with_colour(const char *path, int is_dir, int is_exec);
char *check_for_redirection_and_apply(char *command);
void ping(char *token, char **ptr);
void activity();
void iMan(char *command);
void fg_command(int pid);
void bg_command(int pid);
void neonate_n(int time_arg);
char *get_token_value_from_myshrc(const char *token, const char *myshrc_path);

#endif