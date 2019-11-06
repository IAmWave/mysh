#include "mysh.h"

#include <stdio.h>

void handle_line() { printf("Handling line\n"); }
void handle_command() { printf("Handling command\n"); }
void handle_token(char* token) { printf("Handling token %s\n", token); }

