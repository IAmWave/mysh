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
[a-zA-Z0-9/_\.\-]+   {
  yylval.sval = strdup(yytext);
  return STRING;
}
%%
