%token INT STRING
%token PRINT
%token ID
%token IF ELSE
%token WHILE FOR FUN RETURN LAMBDA
%token NONE;
%token LEN TIME STR ORD
%token BREAK CONTINUE
%right COMMA
%left OR
%left AND
%left NOT EQ NE GT LT GE LE
%right ASSIGN ADDEQ
%left '+' '-'
%left '*' '/' '%'
%{
#include<stdio.h>
#include"tl.h"
#include"list.h"
%}
%%
prog: 
  stmts {
    eval(newEnv(0), $1);
  }
  ;
stmts:
  stmts stmt {
    listPush((List*) $1->data, $2);
    $$ = $1;
  }
  | empty {$1->type = STMTS_TYPE; $$ = $1;}
  ;
stmt:
  exp ';' {
    $$ = $1;
  }
  | IF '(' exp ')' block {
    $$ = newNode2(IF_TYPE, 2, $3, $5);
  }
  | IF '(' exp ')' block ELSE block {
    $$ = newNode2(IF_TYPE, 3, $3, $5, $7);
  }
  | FOR '(' exp_list ';' exp_list ';' exp_list ')' block {
    $$ = newNode2(FOR_TYPE, 4, $3, $5, $7, $9);
  }
  | FOR '(' ID ':' exp ')' block {
    $$ = newNode2(FOREACH_TYPE, 3, $3, $5, $7);
  }
  | WHILE '(' exp ')' block {
    $$ = newNode2(WHILE_TYPE, 2, $3, $5);
  }
  | FUN ID '(' id_list ')' '{' stmts '}' {
    markTailRecursions($7);
    $$ = newNode2(FUN_TYPE, 3, $2, $4, $7);
  }
  | RETURN exp ';' {$$ = newNode2(RETURN_TYPE, 1, $2);}
  | RETURN ';' {$$ = newNode2(RETURN_TYPE, 0);}
  | BREAK ';' {$$ = newNode2(BREAK_TYPE, 0);}
  | CONTINUE ';' {$$ = newNode2(CONTINUE_TYPE, 0);}
  ;

id_list:
  non_empty_id_list {$$ = $1;}
  | empty      { $1->type = ID_LIST_TYPE; $$ = $1; }
  ;

non_empty_id_list:
  non_empty_id_list COMMA ID  { 
    listPush((List*) $1->data, $3);
    $$ = $1;
  }
  | ID         { $$ = newNode2(ID_LIST_TYPE, 1, $1); }
  ;

empty: {$$ = newNode2(-1, 0);}
     ;

block:
  stmt { $$ = $1; }
  | '{' stmts '}' { $$ = $2; }
  ;

exp_list:
  non_empty_exp_list  { $$ = $1; }
  | empty             { $1->type = EXP_LIST_TYPE; $$=$1; }
  ;

non_empty_exp_list:
  non_empty_exp_list COMMA exp  { 
    listPush((List*) $1->data, $3);
    $$ = $1;
  }
  | exp         { $$ = newNode2(EXP_LIST_TYPE, 1, $1); }
  ;

list_access:
  | exp '[' exp ']'    { $$ = newNode2(LIST_ACCESS_TYPE, 2, $1, $3); }
  ;

exp:
  INT { $$ = $1; }
  | STRING { $$ = $1; }
  | LEN '(' exp ')'     { $$ = newNode2(LEN_TYPE, 1, $3); }
  | STR '(' exp ')'     { $$ = newNode2(STR_TYPE, 1, $3); }
  | ORD '(' exp ')'     { $$ = newNode2(ORD_TYPE, 1, $3); }
  | TIME '(' exp ')'    { $$ = newNode2(TIME_TYPE, 1, $3); }
  | NONE                { $$ = newNode2(NONE_TYPE, 0); }
  | LAMBDA '(' id_list ')' '{' stmts '}' { $$ = newNode2(FUN_TYPE, 3, newNode(ID_TYPE, "lambda"), $3, $6); }
  | '[' exp_list ']'    { $$ = newNode2(LIST_TYPE, 1, $2); }
  | list_access         { $$ = $1; }
  | list_access ASSIGN exp { $$ = newNode2(LIST_ASSIGN_TYPE, 3, chld($1, 0), chld($1, 1), $3); }
  | list_access ADDEQ exp  { $$ = newNode2(LIST_ADDEQ_TYPE, 3, chld($1, 0), chld($1, 1), $3); }
  | ID '(' exp_list ')' { $$ = newNode2(CALL_TYPE, 2, $1, $3); }
  | '(' exp ')' { $$ = $2; }
  | exp '+' exp { $$ = newNode2(ADD_TYPE, 2, $1, $3); } 
  | exp '-' exp { $$ = newNode2(SUB_TYPE, 2, $1, $3); } 
  | exp '*' exp { $$ = newNode2(MUL_TYPE, 2, $1, $3); } 
  | exp '/' exp { $$ = newNode2(DIV_TYPE, 2, $1, $3); } 
  | exp '%' exp { $$ = newNode2(MOD_TYPE, 2, $1, $3); } 
  | exp GT exp { $$ = newNode2(GT_TYPE, 2, $1, $3); } 
  | exp LT exp { $$ = newNode2(LT_TYPE, 2, $1, $3); } 
  | exp GE exp { $$ = newNode2(GE_TYPE, 2, $1, $3); } 
  | exp LE exp { $$ = newNode2(LE_TYPE, 2, $1, $3); } 
  | exp EQ exp { $$ = newNode2(EQ_TYPE, 2, $1, $3); } 
  | exp NE exp { $$ = newNode2(NE_TYPE, 2, $1, $3); } 
  | exp AND exp{ $$ = newNode2(AND_TYPE, 2, $1, $3); } 
  | exp OR exp { $$ = newNode2(OR_TYPE, 2, $1, $3); } 
  | NOT exp    { $$ = newNode2(NOT_TYPE, 1, $2); } 
  | ID ASSIGN exp  { $$ = newNode2(ASSIGN_TYPE, 2, $1, $3); }
  | ID ADDEQ exp { $$ = newNode2(ADDEQ_TYPE, 2, $1, $3); }
  | ID          { $$ = $1; }
  | PRINT '(' exp_list ')' {
    $$ = newNode2(PRINT_TYPE, 1, $3);
  }
  ;
%%
int yyerror(char *s) {
  fprintf(stderr, "%s\n",s);
  return 0;
}
int main(){
#if YYDEBUG
  yydebug = 1;
#endif
  yyparse();
  return 0;
}
