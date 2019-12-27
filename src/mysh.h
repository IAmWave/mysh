#ifndef MYSH_H
#define MYSH_H

void handle_line();
void handle_command();
void handle_token(char* token);
void init();
int exit_status;

#endif
