#ifndef MYSH_UTIL_H
#define MYSH_UTIL_H

#include <stdio.h>
#include <unistd.h>

#define eprintf(format, ...) fprintf(stderr, format, ##__VA_ARGS__)

#ifdef DEBUG
#define debugln(format, ...) fprintf(stderr, "%d: " format "\n", getpid(), ##__VA_ARGS__)
#define debug(format, ...) fprintf(stderr, format, ##__VA_ARGS__)
#else
#define debugln(format, ...) ;
#define debug(format, ...) ;
#endif

void* malloc_checked(size_t size);
void update_pwd(char* pwd);

#endif
