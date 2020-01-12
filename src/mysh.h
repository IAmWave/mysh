#ifndef MYSH_H
#define MYSH_H
#include <stdbool.h>

void handle_syntax_error(const char* msg);
void handle_line();
void handle_command();
void handle_token(char* token);
void init();
int exit_status;
bool interactive;

enum redirection_type {
    redirect_in, redirect_out, redirect_out_append
};

#define eprintf(format, ...) fprintf(stderr, format, ##__VA_ARGS__)

#ifdef DEBUG
#define debug(format, ...) fprintf(stderr, "%d: " format, getpid(), ##__VA_ARGS__)
#else
#define debug(format, ...) ;
#endif

#endif
