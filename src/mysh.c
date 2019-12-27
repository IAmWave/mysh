#include "mysh.h"

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>

const int N_TOKENS = 100;

bool interactive;
char* tokens[N_TOKENS];
bool process_running = false;
int exit_status = 0;
int line_number = 0;
char pwd[MAXPATHLEN], oldpwd[MAXPATHLEN];

void clear_command_tokens() {
    for (int i = 0; i < N_TOKENS; i++) {
        free(tokens[i]);
    }
    memset(tokens, 0, sizeof(tokens));
}

void handle_syntax_error(const char* msg) {
    printf("Parsing error on line %d. Message from Bison: %s\n", line_number, msg);
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

int get_n_tokens() {
    int n_tokens;
    for (n_tokens = 0; tokens[n_tokens] != NULL; n_tokens++)
        ;
    return n_tokens;
}

void run_exit() {
    int n_tokens = get_n_tokens();
    if (n_tokens > 1) {
        eprintf("'exit' was not expecting any arguments.\n");
        exit_status = 1;
    } else {
        exit(exit_status);
    }
}

void update_pwd() {
    char* getwd_res = getwd(pwd);
    if (getwd_res == NULL) {
        // getwd stores the error message in the buffer passed as an argument
        eprintf("Error in getwd: %s\n", pwd);
        // This would leave the shell in a weird state, so let's just give up.
        exit(1);
    }
}

void run_cd() {
    int n_tokens = get_n_tokens();
    char* target;
    if (n_tokens == 1) {
        target = getenv("HOME");
    } else if (n_tokens == 2) {
        if (strcmp("-", tokens[1]) == 0) {
            if (strcmp(oldpwd, "") == 0) {
                eprintf("No previous directory.\n");
                exit_status = 1;
                return;
            }
            target = oldpwd;
            printf("%s\n", oldpwd);
        } else {
            target = tokens[1];
        }
    } else {
        eprintf("cd expected at most one argument.\n");
        exit_status = 1;
        return;
    }
    debug("cd from %s to %s, oldpwd=%s\n", pwd, target, oldpwd);
    int res = chdir(target);
    strcpy(oldpwd, pwd);
    if (res != 0) {
        eprintf("Error in cd. errno: %d\n", errno);
        exit_status = 1;
    } else {
        exit_status = 0;
    }
    update_pwd();
}

void run_command() {
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

void handle_command() {
    debug("Handling command\n");
    for (int i = 0; tokens[i] != NULL; i++) {
        debug("token %d: %s\n", i, tokens[i]);
    }
    if (strcmp(tokens[0], "exit") == 0) {
        run_exit();
    } else if (strcmp(tokens[0], "cd") == 0) {
        run_cd();
    } else {
        run_command();
    }
    clear_command_tokens();
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
    update_pwd();
    handle_line();
}
