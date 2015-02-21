#ifndef __PARSER_H__
#define __PARSER_H__
#include "list.h"
#include "ast.h"
#include "tokenizer.h"

Node* parse(List* tokens);
Node* post_process(Node* tree);
Node* match(List* t, int* ip, Token_t type);

Node* stmts(List* t, int* ip);
Node* stmt(List* t, int* ip);

Node* expr0(List* t, int* ip);
Node* expr(List* t, int* ip);
Node* block(List* t, int* ip);
void printAst(Node* ast);

#endif
