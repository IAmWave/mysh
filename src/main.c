#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// stuff from flex
extern int yylex();
extern int yyparse();
extern FILE *yyin;

typedef struct yy_buffer_state *YY_BUFFER_STATE;
void yyerror(const char *s);
extern YY_BUFFER_STATE yy_scan_string(char *str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);

#include "../build/mysh.tab.h"
#include "mysh.h"

int main(int argc, char *argv[]) {
    int opt;
    char *string_to_run = NULL;
    char *file_to_run = NULL;
    while ((opt = getopt(argc, argv, "c:")) != -1) {
        switch (opt) {
            case 'c':
                string_to_run = malloc(strlen(optarg) + 1);
                if (string_to_run == NULL) {
                    eprintf("Error when malloc'ing\n");
                    exit(1);
                }
                strcpy(string_to_run, optarg);
                break;
            case '?':
                fprintf(stderr, "usage: %s [-c string]\n", basename(argv[0]));
                break;
        }
    }
    if (string_to_run == NULL && optind < argc) {
        file_to_run = argv[optind];
    }

    interactive = string_to_run == NULL && file_to_run == NULL;
    init();

    if (file_to_run != NULL) {
        FILE *f = fopen(file_to_run, "r");
        if (f == NULL) {
            eprintf("Could not open file %s\n", file_to_run);
            exit(1);
        }
        yyin = f;
        yyparse();
        fclose(f);
    } else if (string_to_run != NULL) {
        YY_BUFFER_STATE bs = yy_scan_string(string_to_run);
        yyparse();
        free(string_to_run);
        yy_delete_buffer(bs);
    } else {
        yyparse();
    }

    return exit_status;
}
