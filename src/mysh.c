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

bool interactive;
char** tokens;
bool process_running = false;
int exit_status = 0;
int line_number = 0;
char pwd[MAXPATHLEN], oldpwd[MAXPATHLEN];

struct node {
    char* token;
    TAILQ_ENTRY(node) nodes;
};

TAILQ_HEAD(head_s, node) tokens_head;

void clear_command_tokens() {
    struct node* e = NULL;
    while (!TAILQ_EMPTY(&tokens_head)) {
        e = TAILQ_FIRST(&tokens_head);
        TAILQ_REMOVE(&tokens_head, e, nodes);
        free(e->token);
        free(e);
        e = NULL;
    }
}

int get_n_tokens() {
    struct node* e = NULL;
    int n_tokens = 0;
    TAILQ_FOREACH(e, &tokens_head, nodes) { n_tokens++; }
    return n_tokens;
}

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
    char* getcwd_res = getcwd(pwd, MAXPATHLEN);
    if (getcwd_res == NULL) {
        // getcwd stores the error message in the buffer passed as an argument
        eprintf("Error in getcwd: %s\n", pwd);
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
        eprintf("cd: too many arguments\n");
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
        execvp(tokens[0], tokens);
        if (errno == ENOENT) {
            eprintf("%s: No such file or directory\n", tokens[0]);
            exit(127);
        } else {
            eprintf("Unknown error in execvp. errno: %d\n", errno);
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
}

void handle_pipeline() {
    debug("Handling pipeline\n");
    return;
}

void handle_redirection(enum redirection_type type, char* path) {
    debug("Handling redirection %s %d\n", path, type);
}

void handle_command() {
    debug("Handling command\n");
    return;
    int n_tokens = get_n_tokens();
    tokens = calloc(n_tokens + 1, sizeof(char*));
    struct node* e = NULL;
    int qi = 0;
    TAILQ_FOREACH(e, &tokens_head, nodes) {
        debug("Token in queue: %s\n", e->token);
        tokens[qi] = e->token;
        qi++;
    }
    if (strcmp(tokens[0], "exit") == 0) {
        run_exit();
    } else if (strcmp(tokens[0], "cd") == 0) {
        run_cd();
    } else {
        run_command();
    }
    free(tokens);
    clear_command_tokens();
}

void handle_token(char* token) {
    debug("Handling token %s\n", token);
    return;
    struct node* e = NULL;
    e = malloc(sizeof(struct node));
    if (e == NULL) {
        eprintf("Error in malloc when adding token\n");
        exit(1);
    }

    e->token = malloc(strlen(token) + 1);
    strcpy(e->token, token);
    TAILQ_INSERT_TAIL(&tokens_head, e, nodes);
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
    TAILQ_INIT(&tokens_head);
    handle_line();
}
