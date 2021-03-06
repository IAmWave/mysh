#ifndef MYSH_COMMAND_H
#define MYSH_COMMAND_H

#include <stdbool.h>
#include <sys/queue.h>

struct TokenNode {
    char* token;
    TAILQ_ENTRY(TokenNode) nodes;
};

struct Command {
    char *in, *out;
    bool append_out;
    TAILQ_HEAD(head_command_t, TokenNode) tokens_head;
};

void initialize_command(struct Command* cmd);
void free_command(struct Command* cmd);
void add_token(struct Command* cmd, char* token);
int run_command(struct Command* cmd, int exit_status, char* pwd, char* oldpwd);
int get_n_tokens_in_command(struct Command* cmd);

#endif
