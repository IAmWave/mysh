%option noyywrap
%{
  #include "mysh.tab.h"
%}
%x COMMENT
%%
[ \t]          ;
#              BEGIN(COMMENT);
<COMMENT>.     ;
<COMMENT>\n    { BEGIN(INITIAL); return ENDL; }
\n             return ENDL;
;              return SEMICOLON;
\|             return PIPE;
\<             return REDIRECT_IN;
\>             return REDIRECT_OUT;
\>\>           return REDIRECT_OUT_APPEND;
[a-zA-Z0-9/_\.\-]+   {
  yylval.sval = strdup(yytext);
  return STRING;
}
%%
