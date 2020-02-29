#ifndef MYSH_PIPELINE_H
#define MYSH_PIPELINE_H

#include <sys/queue.h>

#include "command.h"

struct CommandNode {
    struct Command* command;
    TAILQ_ENTRY(CommandNode) nodes;
};

struct Pipeline {
    TAILQ_HEAD(head_pipeline_t, CommandNode) commands_head;
};

void initialize_pipeline(struct Pipeline* p);
void free_pipeline(struct Pipeline* p);
void add_command(struct Pipeline* p, struct Command* cmd);
int run_pipeline(struct Pipeline* p, int exit_status, char* pwd, char* oldpwd);
int get_n_commands_in_pipeline(struct Pipeline* p);

#endif
