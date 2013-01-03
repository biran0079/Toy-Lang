D       [0-9]
L       [a-zA-Z]
%{
#include "tl.h"
#include "tl.tab.h"
%}
%%
print       {return PRINT;}
if         {return IF;}
else       {return ELSE;}
while     {return WHILE;}
break     {return BREAK;}
continue  {return CONTINUE;}
fun       {return FUN;}
lambda       {return LAMBDA;}
return    {return RETURN;}
len       {return LEN;}
{D}+        {yylval = newNode(INT_TYPE, (void*)atoi(yytext)); return INT;}
{L}({D}|{L})*   {yylval = newNode(ID_TYPE, copy(yytext, 1+strlen(yytext))); return ID;}
">="         {return GE;}
"<="         {return LE;}
"=="         {return EQ;}
"="          {return ASSIGN;}
"!="         {return NE;}
"&&"         {return AND;}
"||"         {return OR;}
"!"          {return NOT;}
">"          {return GT;}
"<"          {return LT;}
[-+*/%();{},\[\]]  {return *yytext;}
[ \t\n]     ;
%%
int yywrap(){
  return 1;  
}
