#include "command.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
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

void free_command(struct Command* cmd) {
    free(cmd->in);
    free(cmd->out);
    while (!TAILQ_EMPTY(&(cmd->tokens_head))) {
        struct TokenNode* e = TAILQ_FIRST(&(cmd->tokens_head));
        TAILQ_REMOVE(&(cmd->tokens_head), e, nodes);
        free(e->token);
        free(e);
    }
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

int run_regular_command(int n_tokens, char** tokens, int fd_in, int fd_out) {
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
        if (fd_in != STDIN_FILENO) {
            if (close(STDIN_FILENO) == -1) err(1, "close");
            if (dup(fd_in) == -1) err(1, "dup");
        }
        if (fd_out != STDOUT_FILENO) {
            if (close(STDOUT_FILENO) == -1) err(1, "close");
            if (dup(fd_out) == -1) err(1, "dup");
        }
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
        pid_t pid = wait(&stat_loc);
        if (pid == -1) {
            err(1, "wait");
        }
        if (WIFSIGNALED(stat_loc)) {
            eprintf("Killed by signal %d\n", WTERMSIG(stat_loc));
            return 128 + WTERMSIG(stat_loc);
        } else {
            return WEXITSTATUS(stat_loc);
        }
    }
}

int open_redirections(struct Command* cmd, int* fd_in, int* fd_out) {
    if (cmd->in != NULL) {
        *fd_in = open(cmd->in, O_RDONLY);
        if (*fd_in == -1) {
            eprintf("Error when open()-ing %s: %s\n", cmd->in, strerror(errno));
            return 1;
        }
        debug("Input from %s\n", cmd->in);
    }
    if (cmd->out != NULL) {
        *fd_out = open(cmd->out,
                       O_WRONLY | O_CREAT | (cmd->append_out ? O_APPEND : O_TRUNC), 0664);
        if (*fd_out == -1) {
            eprintf("Error when open()-ing %s: %s\n", cmd->out, strerror(errno));
            return 1;
        }
        debug("Output to %s\n", cmd->out);
    }
    return 0;
}

void close_redirections(int fd_in, int fd_out) {
    if (fd_in != STDIN_FILENO) {
        if (close(fd_in) == -1) {
            err(1, "close");
        }
    }
    if (fd_out != STDOUT_FILENO) {
        if (close(fd_out) == -1) {
            err(1, "close");
        }
    }
}

int run_command(struct Command* cmd, int exit_status, char* pwd, char* oldpwd) {
    struct TokenNode* e = NULL;
    debug("Running command ");
    TAILQ_FOREACH(e, &(cmd->tokens_head), nodes) { debug(" %s", e->token); }
    debug("\n");

    int fd_in = STDIN_FILENO, fd_out = STDOUT_FILENO;
    if (open_redirections(cmd, &fd_in, &fd_out) != 0) {
        return 1;
    }
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
        exit_status = run_regular_command(n_tokens, tokens, fd_in, fd_out);
    }
    close_redirections(fd_in, fd_out);
    free(tokens);
    return exit_status;
}

int get_n_tokens_in_command(struct Command* cmd) {
    struct TokenNode* e = NULL;
    int n_tokens = 0;
    TAILQ_FOREACH(e, &(cmd->tokens_head), nodes) { n_tokens++; }
    return n_tokens;
}
