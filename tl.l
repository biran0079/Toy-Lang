D       [0-9]
L       [a-zA-Z]
%{
#include "core.h"
#include "ast.h"
#include "util.h"
#include "hashTable.h"
#include "tl.tab.h"

extern HashTable* idToIntMap;

%}

%%
if         {return IF;}
else       {return ELSE;}
while     {return WHILE;}
break     {return BREAK;}
continue  {return CONTINUE;}
fun       {return FUN;}
for       {return FOR;}
lambda    {return LAMBDA;}
return    {return RETURN;}
time       {return TIME;}
none       {return NONE;}
try       {return TRY;}
catch       {return CATCH;}
finally       {return FINALLY;}
throw       {return THROW;}
local       {return LOCAL;}
import       {return IMPORT;}
{D}+        {yylval = newNode(INT_TYPE, (void*) strToLong(yytext)); return INT;}
{L}({D}|{L})*   {
  yylval = newNode(ID_TYPE, (void*) getIntId(yytext));
  return ID;
}
\"(\\.|[^\"])*\"  {yylval = newNode(STRING_TYPE, literalStringToString(yytext)); return STRING;}
"++"         {return ADDADD;}
"+="         {return ADDEQ;}
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
","          {return COMMA;}
[-+*/%();{}\[\]:.]  {return *yytext;}
[ \t\n\r]     ;
.            {printf("unknown character for lex: %s\n", yytext);exit(-1);}
%%
int yywrap() {
    return 1;
}
