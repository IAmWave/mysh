%{
    #include <stdio.h>
    #include <stdlib.h>

    #include "../src/mysh.h"
    // stuff from flex that bison needs to know about:
    extern int yylex();
    extern int yyparse();
    extern FILE *yyin;
 
    void yyerror(const char *s);
%}

%union {
    char *sval;
}

// define the constant-string tokens:
%token ENDL SEMICOLON PIPE

// define the "terminal symbol" token types I'm going to use (in CAPS
// by convention), and associate each with a field of the union:
//%token <ival> INT
//%token <fval> FLOAT
%token <sval> STRING

%%

// the first rule defined is the highest-level rule
lines:
    lines endl line
|   line
|   error
    ;

line:
    /*empty*/ { /*handle_line();*/ }
|   SEMICOLON { /*handle_line();*/ }
|   commands { /*handle_line();*/ }
|   commands SEMICOLON { /*handle_line();*/ }
    ;

endl: ENDL
    {
        handle_line();
    };

commands:
    command { handle_command(); }
|   commands SEMICOLON command { handle_command(); }
    ;

command:
    command_token
|   command command_token
    ;

command_token: STRING
    {
        handle_token($1);
        //printf("token %s\n", $1);
        free($1);
    };

%%

void yyerror(const char *msg) {
    handle_syntax_error(msg);
}
