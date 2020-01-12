#include "command.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <unistd.h>

#include "util.h"

void initialize_command(struct Command* cmd) {
    cmd->in = NULL;
    cmd->out = NULL;
    cmd->append_out = false;
    TAILQ_INIT(&(cmd->tokens_head));
}

void add_token(struct Command* cmd, char* token) {
    struct TokenNode* e = NULL;
    e = malloc_checked(sizeof(struct TokenNode));
    e->token = malloc_checked(strlen(token) + 1);

    strcpy(e->token, token);
    TAILQ_INSERT_TAIL(&(cmd->tokens_head), e, nodes);
}

int run_cd(int n_tokens, char** tokens, char* pwd, char* oldpwd) {
    char* target;
    if (n_tokens == 1) {
        target = getenv("HOME");
    } else if (n_tokens == 2) {
        if (strcmp("-", tokens[1]) == 0) {
            if (strcmp(oldpwd, "") == 0) {
                eprintf("No previous directory.\n");
                return 1;
            }
            target = oldpwd;
            printf("%s\n", oldpwd);
        } else {
            target = tokens[1];
        }
    } else {
        eprintf("cd: too many arguments\n");
        return 1;
    }
    debug("cd from %s to %s, oldpwd=%s\n", pwd, target, oldpwd);
    int res = chdir(target);
    strcpy(oldpwd, pwd);
    update_pwd(pwd);
    if (res != 0) {
        eprintf("Error in cd. errno: %d\n", errno);
        return 1;
    } else {
        return 0;
    }
}

int run_regular_command(int n_tokens, char** tokens) {
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
        if (WIFSIGNALED(stat_loc)) {
            eprintf("Killed by signal %d\n", WTERMSIG(stat_loc));
            return 128 + WTERMSIG(stat_loc);
        } else {
            return WEXITSTATUS(stat_loc);
        }
    }
}

int run_command(struct Command* cmd, int exit_status, char* pwd, char* oldpwd) {
    struct TokenNode* e = NULL;
    debug("Running command ");
    TAILQ_FOREACH(e, &(cmd->tokens_head), nodes) { debug(" %s", e->token); }
    debug("\n");

    int n_tokens = get_n_tokens_in_command(cmd);
    char** tokens = calloc(n_tokens + 1, sizeof(char*));
    int qi = 0;
    TAILQ_FOREACH(e, &(cmd->tokens_head), nodes) {
        // debug("Token in queue: %s\n", e->token);
        tokens[qi] = e->token;
        qi++;
    }
    if (strcmp(tokens[0], "exit") == 0) {
        if (n_tokens > 1) {
            eprintf("'exit' was not expecting any arguments.\n");
            exit_status = 1;
        } else {
            exit(exit_status);
        }
    } else if (strcmp(tokens[0], "cd") == 0) {
        exit_status = run_cd(n_tokens, tokens, pwd, oldpwd);
    } else {
        exit_status = run_regular_command(n_tokens, tokens);
    }
    free(tokens);
    clear_command_tokens(cmd);
    return exit_status;
}

void clear_command_tokens(struct Command* cmd) {
    struct TokenNode* e = NULL;
    while (!TAILQ_EMPTY(&(cmd->tokens_head))) {
        e = TAILQ_FIRST(&(cmd->tokens_head));
        TAILQ_REMOVE(&(cmd->tokens_head), e, nodes);
        free(e->token);
        free(e);
        e = NULL;
    }
}

int get_n_tokens_in_command(struct Command* cmd) {
    struct TokenNode* e = NULL;
    int n_tokens = 0;
    TAILQ_FOREACH(e, &(cmd->tokens_head), nodes) { n_tokens++; }
    return n_tokens;
}
