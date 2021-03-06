#ifndef __PARSER_H__
#define __PARSER_H__
#include "list.h"
#include "ast.h"
#include "tokenizer.h"

Node *parse(List *tokens);

Node *stmts(List *t, int *ip);
Node *stmt(List *t, int *ip);

Node *expr(List *t, int *ip);
Node *block(List *t, int *ip);
Node *expList(List *t, int *ip);
Node *nonEmptyExpList(List *t, int *ip);
Node *idList(List *t, int *ip);
Node *nonEmptyIdList(List *t, int *ip);

#endif
