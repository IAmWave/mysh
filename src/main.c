#include <stdio.h>
#include <stdlib.h>

// stuff from flex
extern int yylex();
extern int yyparse();
extern FILE *yyin;

void yyerror(const char *s);

#include "../build/mysh.tab.h"

int main(int argc, char **argv) {
    //  // open a file handle to a particular file:
    //  FILE *myfile = fopen("a.snazzle.file", "r");
    //  // make sure it's valid:
    //  if (!myfile) {
    //    printf("I can't open a.snazzle.file!"); //, endl;
    //    return -1;
    //  }
    //  // Set flex to read from it instead of defaulting to STDIN:
    //  yyin = myfile;
    //
    // Parse through the input:
    yyparse();
}
