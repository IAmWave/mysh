#ifndef MYSH_UTIL_H
#define MYSH_UTIL_H

#include <stdio.h>

#define eprintf(format, ...) fprintf(stderr, format, ##__VA_ARGS__)

#ifdef DEBUG
#define debug(format, ...) fprintf(stderr, "%d: " format, getpid(), ##__VA_ARGS__)
#else
#define debug(format, ...) ;
#endif

void* malloc_checked(size_t size);
void update_pwd(char* pwd);

#endif
