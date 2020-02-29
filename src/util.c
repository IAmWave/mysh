#include "util.h"

#include <err.h>
#include <stdlib.h>
#include <sys/param.h>
#include <unistd.h>

void* malloc_checked(size_t size) {
    void* res = malloc(size);
    if (res == NULL) {
        err(1, "malloc");
    }
    return res;
}

void update_pwd(char* pwd) {
    char* getcwd_res = getcwd(pwd, MAXPATHLEN);
    if (getcwd_res == NULL) {
        // getcwd stores the error message in the buffer passed as an argument
        eprintf("Error in getcwd: %s\n", pwd);
        // This would leave the shell in a weird state, so let's just give up.
        exit(1);
    }
}
