#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__
#include "list.h"
#include "core.h"

typedef enum {
  INT_T,
  STRING_T,
  ID_T,
  WHILE_T,
  FOR_T,
  FUN_T,
  RETURN_T,
  LAMBDA_T,
  NONE_T,
  LOCAL_T,
  TIME_T,
  BREAK_T,
  CONTINUE_T,
  TRY_T,
  CATCH_T,
  FINALLY_T,
  THROW_T,
  IF_T,
  ELSE_T,
  ADDADD_T,
  IMPORT_T,
  ADD_T,
  SUB_T,
  MUL_T,
  DIV_T,
  MOD_T,
  DOT_T,
  AND_T,
  OR_T,
  COMMA_T,
  NOT_T,
  EQ_T,
  NE_T,
  GT_T,
  LT_T,
  GE_T,
  LE_T,
  ASSIGN_T,
  ADDEQ_T,
  OP_B_T,
  CLO_B_T,
  OP_CB_T,
  CLO_CB_T,
  OP_SB_T,
  CLO_SB_T,
  SEMICOLON_T,
  COLON_T,
  EOF_T,
} Token_t;

struct Token {
  Token_t type;
  void *data;
};

List *tokenize(char *s);

char *tokenTypeToStr(Token_t type);

void freeTokenList(List *l);
#endif
