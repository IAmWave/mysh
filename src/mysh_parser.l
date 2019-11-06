%{
  #include "mysh.tab.h"
%}
%%
\n             return ENDL;
[ \t]          ;
;              { return SEMICOLON; }
\|             { return PIPE; }
[a-zA-Z0-9]+   {
  yylval.sval = strdup(yytext);
  return STRING;
}
%%
