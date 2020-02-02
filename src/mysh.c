#include "mysh.h"

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/queue.h>
#include <unistd.h>

#include "command.h"
#include "util.h"

bool interactive;
char** tokens;
bool process_running = false;
int exit_status = 0;
int line_number = 0;
char pwd[MAXPATHLEN], oldpwd[MAXPATHLEN];
struct Command* cmd;

void handle_syntax_error(const char* msg) {
    eprintf("Parsing error on line %d. Message from Bison: %s\n", line_number, msg);
    exit_status = 2;
    if (!interactive) {
        exit(2);
    }
}

void handle_line() {
    line_number++;
    if (interactive) {
        printf("mysh:%s$ ", pwd);
        fflush(stdout);
    }
}

void handle_pipeline() { debug("Handling pipeline\n"); }

void handle_redirection(enum redirection_type type, char* path) {
    debug("Handling redirection %s %d\n", path, type);
    char** copy_to = NULL;
    switch (type) {
        case redirect_in:
            copy_to = &cmd->in;
            break;
        case redirect_out:
            copy_to = &cmd->out;
            cmd->append_out = false;
            break;
        case redirect_out_append:
            copy_to = &cmd->out;
            cmd->append_out = true;
            break;
    }
    free(*copy_to);
    *copy_to = malloc_checked(strlen(path) + 1);
    strcpy(*copy_to, path);
}

void handle_command() {
    debug("Handling command\n");
    process_running = true;
    exit_status = run_command(cmd, exit_status, pwd, oldpwd);
    free_command(cmd);
    cmd = malloc_checked(sizeof(struct Command));
    initialize_command(cmd);
    process_running = false;
}

void handle_token(char* token) {
    debug("Handling token %s\n", token);
    add_token(cmd, token);
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
    update_pwd(pwd);
    cmd = malloc_checked(sizeof(struct Command));
    initialize_command(cmd);
    handle_line();
}
