#include "mysh.h"

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/queue.h>
#include <sys/wait.h>
#include <unistd.h>

#include "util.h"
#include "command.h"

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

void handle_pipeline() {
    debug("Handling pipeline\n");
}

void handle_redirection(enum redirection_type type, char* path) {
    debug("Handling redirection %s %d\n", path, type);
}

void handle_command() {
    debug("Handling command\n");
    process_running = true;
    exit_status = run_command(cmd, exit_status, pwd, oldpwd);
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
