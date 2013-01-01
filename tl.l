D       [0-9]
L       [a-zA-Z]
%{
#include "tl.h"
#include "tl.tab.h"
%}
%%
print       {return PRINT;}
{D}+        {yylval = newNode(INT_TYPE, (void*)atoi(yytext)); return INT;}
{L}({D}|{L})*   {yylval = newNode(ID_TYPE, copy(yytext, 1+strlen(yytext))); return ID;}
[-+=*/();]         {return *yytext;}
[ \t\n]     ;
%%
int yywrap(){
  return 1;  
}
