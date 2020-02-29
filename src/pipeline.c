#include "pipeline.h"

#include <err.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <unistd.h>

#include "util.h"

void initialize_pipeline(struct Pipeline* p) { TAILQ_INIT(&(p->commands_head)); }

void free_pipeline(struct Pipeline* p) {
    while (!TAILQ_EMPTY(&(p->commands_head))) {
        struct CommandNode* e = TAILQ_FIRST(&(p->commands_head));
        TAILQ_REMOVE(&(p->commands_head), e, nodes);
        free_command(e->command);
        free(e);
    }
}

void add_command(struct Pipeline* p, struct Command* cmd) {
    struct CommandNode* e = NULL;
    e = malloc_checked(sizeof(struct CommandNode));
    e->command = cmd;
    TAILQ_INSERT_TAIL(&(p->commands_head), e, nodes);
}

int run_pipeline(struct Pipeline* p, int exit_status, char* pwd, char* oldpwd) {
    debugln("Running pipeline");
    int n_commands = get_n_commands_in_pipeline(p);
    if (n_commands == 1) {
        struct Command* cmd = TAILQ_FIRST(&(p->commands_head))->command;
        return run_command(cmd, exit_status, pwd, oldpwd);
    } else {
        debugln("Multiple commands");
        struct CommandNode* e = NULL;

        int i = 0;
        int in_fd = STDIN_FILENO;
        TAILQ_FOREACH(e, &(p->commands_head), nodes) {
            int out_fd, in_fd_next;
            if (i == n_commands - 1) {
                out_fd = STDOUT_FILENO;
            } else {
                int pipe_fds[2];
                if (pipe(pipe_fds) == -1) err(1, "pipe");
                in_fd_next = pipe_fds[0];
                out_fd = pipe_fds[1];
            }

            int pid;
            switch (pid = fork()) {
                case -1:
                    err(1, "fork");
                    break;
                case 0:  // child
                    // printf("%d %d\n", in_fd, out_fd);
                    if (in_fd != STDIN_FILENO) {
                        if (close(STDIN_FILENO) == -1) err(1, "close");
                        if (dup(in_fd) == -1) err(1, "dup");
                        if (close(in_fd) == -1) err(1, "close");
                    }
                    if (out_fd != STDOUT_FILENO) {
                        if (close(STDOUT_FILENO) == -1) err(1, "close");
                        if (dup(out_fd) == -1) err(1, "dup");
                        if (close(out_fd) == -1) err(1, "close");
                    }
                    run_command(e->command, exit_status, pwd, oldpwd);
                    exit(0);
                    break;
                default:  // parent
                    break;
            }
            if (in_fd != STDIN_FILENO) {
                if (close(in_fd) == -1) err(1, "close");
            }
            if (out_fd != STDOUT_FILENO) {
                if (close(out_fd) == -1) err(1, "close");
            }
            in_fd = in_fd_next;
            i++;
        }
        int status, wpid;
        while ((wpid = wait(&status)) > 0)
            ;  // this way, the parent waits for all the child processes
    }
    return exit_status;
}

int get_n_commands_in_pipeline(struct Pipeline* p) {
    struct CommandNode* e = NULL;
    int n_commands = 0;
    TAILQ_FOREACH(e, &(p->commands_head), nodes) { n_commands++; }
    return n_commands;
}
