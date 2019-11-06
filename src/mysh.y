%{
  #include <stdio.h>
  #include <stdlib.h>

  // stuff from flex that bison needs to know about:
  extern int yylex();
  extern int yyparse();
  extern FILE *yyin;
 
  void yyerror(const char *s);
%}

%union {
  // int ival;
  // float fval;
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

// the first rule defined is the highest-level rule, which in our
// case is just the concept of a whole "snazzle file":
lines:
  lines ENDL
| lines SEMICOLON ENDL
| lines pipelines
| pipelines
  ;
pipelines:
  pipeline0
| pipelines SEMICOLON pipeline0
  {
    printf("eol\n");
  }  
  ;

pipeline0:
  pipeline
  {
    printf("-------ppl\n");
  }
  ;
pipeline:
  command
| pipeline PIPE command 
  {
    printf("pipeline\n");
    // printf("pipeline %s\n", $1);
  }
  ;

command:
  command_token
| command command_token
  ;
command_token:
  STRING
  {
    printf("command %s\n", $1);
  }
  ;

// template:
//   typelines
//   ;
// typelines:
//   typelines typeline
//   | typeline
//   ;
// typeline:
//   TYPE STRING {
//       printf("new defined snazzle type: %s\n", $2); //, $2, endl;
//       free($2);
//     }
//   ;
// body_section:
//   body_lines
//   ;
// body_lines:
//   body_lines body_line
//   | body_line
//   ;
// body_line:
//   INT INT INT INT STRING {
//       printf("new snazzle: %d, %d, %d, %d, %s\n", $1, $2, $3, $4, $5); 
//       free($5);
//     }
//   ;
// footer:
//   END
//   ;

%%

int main(int argc, char** argv) {
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

void yyerror(const char *s) {
  printf("EEK, parse error!  Message: %s\n", s); //, s, endl;
  // might as well halt now:
  exit(-1);
}
