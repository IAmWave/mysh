#include "mysh.h"

#include <errno.h>
#include <signal.h>
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

#define eprintf(format, ...) fprintf(stderr, format, ##__VA_ARGS__)

const char* PROMPT = "mysh$ ";
const int N_TOKENS = 100;

char* tokens[N_TOKENS];
bool process_running = false;
int exit_status = 0;
int line_number = 0;

void clear_command_tokens() {
    for (int i = 0; i < N_TOKENS; i++) {
        free(tokens[i]);
    }
    memset(tokens, 0, sizeof(tokens));
}

void handle_syntax_error(char* msg) {
    printf("Syntax error on line %d\n", line_number);
    exit_status = 2;
}

void handle_line() {
    line_number++;
    printf("%s", PROMPT);
    fflush(stdout);
}

void handle_command() {
    debug("Handling command\n");
    for (int i = 0; tokens[i] != NULL; i++) {
        debug("token %d: %s\n", i, tokens[i]);
    }
    pid_t fork_pid;
    bool parent;
    process_running = true;
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
        if (errno == ENOENT) {
            eprintf("%s: No such file or directory\n", tokens[0]);
            exit(127);
        } else {
            eprintf("Unknown error in execvp. errno: %d\n", res, errno);
            exit(-1);
        }
    } else {
        clear_command_tokens();
        int stat_loc;
        wait(&stat_loc);
        process_running = false;
        if (WIFSIGNALED(stat_loc)) {
            eprintf("Killed by signal %d\n", WTERMSIG(stat_loc));
            exit_status = 128 + WTERMSIG(stat_loc);
        } else {
            exit_status = WEXITSTATUS(stat_loc);
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

void sigint_handler() {
    if (!process_running) {
        write(1, "\n", strlen("\n"));
        handle_line();
    }
}

void set_sigint_handler() {
    struct sigaction act;
    act.sa_handler = sigint_handler;
    act.sa_flags = SA_RESTART;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, NULL);
}

void init() {
    set_sigint_handler();
    handle_line();
}
