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
%token ENDL SEMICOLON PIPE REDIRECT_IN REDIRECT_OUT REDIRECT_OUT_APPEND

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

line:
    /*empty*/ { /*handle_line();*/ }
|   SEMICOLON { /*handle_line();*/ }
|   pipelines { /*handle_line();*/ }
|   pipelines SEMICOLON { /*handle_line();*/ }

endl: ENDL
    {
        handle_line();
    }

pipelines:
    pipeline { handle_pipeline(); }
|   pipelines SEMICOLON pipeline { handle_pipeline(); }

pipeline:
    command_redirected { handle_command(); }
|   pipeline PIPE command_redirected { handle_command(); }

command_redirected:
    command
|   command_redirected REDIRECT_IN STRING
    {
        handle_redirection(redirect_in, $3);
        free($3);
    }
|   command_redirected REDIRECT_OUT STRING
    {
        handle_redirection(redirect_out, $3);
        free($3);
    }
|   command_redirected REDIRECT_OUT_APPEND STRING
    {
        handle_redirection(redirect_out_append, $3);
        free($3);
    }

command:
    command_token
|   command command_token

command_token: STRING
    {
        handle_token($1);
        free($1);
    }

%%

void yyerror(const char *msg) {
    handle_syntax_error(msg);
}
