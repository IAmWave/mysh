#ifndef MYSH_H
#define MYSH_H

#include <stdbool.h>

enum redirection_type { redirect_in, redirect_out, redirect_out_append };

void handle_syntax_error(const char* msg);
void handle_line();
void handle_pipeline();
void handle_command();
void handle_redirection(enum redirection_type type, char* path);
void handle_token(char* token);
void init();
int exit_status;
bool interactive;

#endif
