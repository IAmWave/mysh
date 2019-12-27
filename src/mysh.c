#include "mysh.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef DEBUG
#define debug(format, ...) fprintf(stderr, "%d: " format, getpid(), ##__VA_ARGS__)
#else
#define debug(format, ...) ;
#endif

const char* PROMPT = "mysh$ ";
const int N_TOKENS = 100;
char* tokens[N_TOKENS];

void clear_command_tokens() {
    for (int i = 0; i < N_TOKENS; i++) {
        free(tokens[i]);
    }
    memset(tokens, 0, sizeof(tokens));
}

void handle_line() {
    // printf("\n");
    printf("%s", PROMPT);
}

void handle_command() {
    debug("Handling command\n");
    for (int i = 0; tokens[i] != NULL; i++) {
        debug("token %d: %s\n", i, tokens[i]);
    }
    pid_t fork_pid;
    bool parent;
    switch (fork_pid = fork()) {
        case -1:
            debug("Error when forking\n");
            exit(1);
        case 0:
            parent = false;
            break;
        default:
            parent = true;
    }
    if (!parent) {
        int res = execvp(tokens[0], tokens);
        debug("Error occurred in execvp: %d\n", res);
        exit(1);
    } else {
        clear_command_tokens();
        int stat_loc;
        wait(&stat_loc);
        if (WIFSIGNALED(stat_loc)) {
            printf("Child terminated with signal %d\n", WTERMSIG(stat_loc));
        }
    }
    // char* args[] = {"1", "x", NULL};
    // printf("x %d\n", res);
}
void handle_token(char* token) {
    debug("Handling token %s\n", token);
    int i;
    for (i = 0; tokens[i] != NULL; i++)
        ;
    tokens[i] = malloc(strlen(token) + 1);
    strcpy(tokens[i], token);
}

void init() { handle_line(); }
