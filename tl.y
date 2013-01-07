%token INT STRING
%token PRINT
%token ID
%token WHILE FOR FUN RETURN LAMBDA
%token NONE
%token LOCAL
%token LEN TIME STR ORD
%token BREAK CONTINUE
%token TRY CATCH FINALLY THROW
%token IF ELSE
%token ADDADD

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
Node* parseTree;
%}

%%
prog: 
  stmts {
    parseTree = $1;
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
  | LOCAL id_list ';' { $$ = newNode2(LOCAL_TYPE, 1, $2); }
  | TRY block CATCH '(' ID ')' block { $$ = newNode2(TRY_TYPE, 3, $2, $5, $7); }
  | TRY block CATCH '(' ID ')' block FINALLY block { $$ = newNode2(TRY_TYPE, 4, $2, $5, $7, $9); }
  | THROW exp ';'  { $$ = newNode2(THROW_TYPE, 1, $2); }
  | RETURN exp ';' {$$ = newNode2(RETURN_TYPE, 1, $2);}
  | RETURN ';' {$$ = newNode2(RETURN_TYPE, 1, newNode2(NONE_TYPE, 0));}
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
  list_exp '[' exp ']'    { $$ = newNode2(LIST_ACCESS_TYPE, 2, $1, $3); }
  | general_exp '[' exp ']'    { $$ = newNode2(LIST_ACCESS_TYPE, 2, $1, $3); }
  | string_exp '[' exp ']'    { $$ = newNode2(LIST_ACCESS_TYPE, 2, $1, $3); }
  ;

left_value:
  list_access  { $$ = $1; }
  | ID         { $$ = $1; }

int_exp:
  INT { $$ = $1; }
  | ID ADDADD  { $$ = newNode2(ADDADD_TYPE, 1, $1); }
  | LEN '(' exp ')' { $$ = newNode2(LEN_TYPE, 1, $3); }
  | exp GT exp { $$ = newNode2(GT_TYPE, 2, $1, $3); } 
  | exp LT exp { $$ = newNode2(LT_TYPE, 2, $1, $3); } 
  | exp GE exp { $$ = newNode2(GE_TYPE, 2, $1, $3); } 
  | exp LE exp { $$ = newNode2(LE_TYPE, 2, $1, $3); } 
  | exp EQ exp { $$ = newNode2(EQ_TYPE, 2, $1, $3); } 
  | exp NE exp { $$ = newNode2(NE_TYPE, 2, $1, $3); } 
  | exp AND exp{ $$ = newNode2(AND_TYPE, 2, $1, $3); } 
  | exp OR exp { $$ = newNode2(OR_TYPE, 2, $1, $3); } 
  | NOT exp    { $$ = newNode2(NOT_TYPE, 1, $2); } 
  | exp '-' exp { $$ = newNode2(SUB_TYPE, 2, $1, $3); } 
  | exp '*' exp { $$ = newNode2(MUL_TYPE, 2, $1, $3); } 
  | exp '/' exp { $$ = newNode2(DIV_TYPE, 2, $1, $3); } 
  | exp '%' exp { $$ = newNode2(MOD_TYPE, 2, $1, $3); } 
  ;

string_exp:
  STRING { $$ = $1; }
  | STR '(' exp ')'     { $$ = newNode2(STR_TYPE, 1, $3); }
  | ORD '(' exp ')'     { $$ = newNode2(ORD_TYPE, 1, $3); }
  ;

fun_exp:
  LAMBDA '(' id_list ')' '{' stmts '}' { $$ = newNode2(FUN_TYPE, 3, newNode(ID_TYPE, "lambda"), $3, $6); }
  ;
  
list_exp:
  '[' exp_list ']'    { $2->type = LIST_TYPE; $$ = $2; }
  ;

none_exp:
  NONE                { $$ = newNode2(NONE_TYPE, 0); }
  | PRINT '(' exp_list ')' {
    $$ = newNode2(PRINT_TYPE, 1, $3);
  }
  ;

general_exp:
  TIME '(' exp ')'    { $$ = newNode2(TIME_TYPE, 1, $3); }
  | list_access         { $$ = $1; }
  | ID '(' exp_list ')' { $$ = newNode2(CALL_TYPE, 2, $1, $3); }
  | '(' exp ')' { $$ = $2; }
  | exp '+' exp { $$ = newNode2(ADD_TYPE, 2, $1, $3); } 
  | left_value ASSIGN exp  { $$ = newNode2(ASSIGN_TYPE, 2, $1, $3); }
  | left_value ADDEQ exp { $$ = newNode2(ADDEQ_TYPE, 2, $1, $3); }
  | ID          { $$ = $1; }
  ;

exp:
  int_exp  { $$ = $1; }
  | string_exp { $$ = $1; }
  | fun_exp { $$ = $1; }
  | list_exp { $$ = $1; }
  | none_exp { $$ = $1; }
  | general_exp { $$ = $1; }
  ;

%%
int yyerror(char *s) {
  fprintf(stderr, "%s\n",s);
  return 0;
}
void help(){
  fprintf(stderr, "Usage: tl [<options>] <filename>\n");
  fprintf(stderr, "\t-d\tinstead of evaluating the program, it converts tabstract syntax tree to dot language, \n"
                  "\t\twhich can be compiled to image using dot tool\n"
                  "\t-l\tlist number of created objects\n");
  exit(-1);
}
int main(int argc, char** argv){
#if YYDEBUG
  yydebug = 1;
#endif
  int toDot = 0;
  int listCreatedObj = 0;
  int i;
  char* src = 0;
  for(i=1; i<argc;i++) {
    if(argv[i][0]=='-') {
      switch(argv[i][1]) {
        case 'd' : toDot = 1;break;
        case 'l' : listCreatedObj = 1; break;
        default: help();
      }
    } else {
      src = argv[i];
    }
  }
  if(src){
    if(0 == freopen(src, "r", stdin)) {
      error("cannot open input file\n");
    }
  } else {
    src = "stdin";
  }
  yyparse();
  if(toDot) {
    int l = strlen(src);
    char* s = (char*) malloc(l + 4);
    s[0]=0;
    strcpy(s, src);
    strcat(s, ".dot");
    FILE* f=fopen(s, "w");
    nodeToDot(f, parseTree);
    fclose(f);
  } else {
    eval(newEnv(0), parseTree);
  }
  if(listCreatedObj) {
    listCreatedObjectsCount();
  }
  return 0;
}
