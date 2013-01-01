%token INT
%token PRINT
%token ID
%left '+' '-'
%left '*' '/'
%{
#include<stdio.h>
#include"tl.h"
#include"list.h"
%}
%%
prog: 
  stmts {
    eval(newEnv(), $1);
  }
  ;
stmts:
  stmts stmt {
    listAdd((List*) $1->data, $2);
    $$ = $1;
  }
  | {$$ = newNode2(STMTS_TYPE, 0);}
  ;
stmt:
  exp ';' {
    $$ = $1;
  }
  ;
exp:
  INT { $$ = $1; }
  | '(' exp ')' { $$ = $2; }
  | exp '+' exp { $$ = newNode2(ADD_TYPE, 2, $1, $3); } 
  | exp '-' exp { $$ = newNode2(SUB_TYPE, 2, $1, $3); } 
  | exp '*' exp { $$ = newNode2(MUL_TYPE, 2, $1, $3); } 
  | exp '/' exp { $$ = newNode2(DIV_TYPE, 2, $1, $3); } 
  | ID '=' exp  { $$ = newNode2(ASSIGN_TYPE, 2, $1, $3); }
  | ID          { $$ = $1; }
  | PRINT '(' exp ')' {
    $$ = newNode(PRINT_TYPE, $3);
  }
  ;
%%
int yyerror(char *s) {
  fprintf(stderr, "%s\n",s);
  return 0;
}
int main(){
  yyparse();
  return 0;
}
