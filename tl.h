#ifndef _TL_H_
#define _TL_H_

#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<assert.h>
#include<setjmp.h>
#include"hash_table.h"
#include"list.h"
#define MALLOC(T) (T*)malloc(sizeof(T))
typedef enum {
  INT_TYPE,
  ID_TYPE,
  PRINT_TYPE,
  STMTS_TYPE,
  ASSIGN_TYPE,
  ADD_TYPE,
  SUB_TYPE,
  MUL_TYPE,
  DIV_TYPE,
  IF_TYPE,
  GT_TYPE,
  LT_TYPE,
  GE_TYPE,
  LE_TYPE,
  EQ_TYPE,
  NE_TYPE,
  AND_TYPE,
  OR_TYPE,
  NOT_TYPE,
  WHILE_TYPE,
  ARGS_TYPE,
  FUN_TYPE,
  EXP_LIST_TYPE,
  ID_LIST_TYPE,
  APP_TYPE,
  RETURN_TYPE,
  BREAK_TYPE,
  CONTINUE_TYPE,
  MOD_TYPE,
} NodeType;

typedef enum {
  INT_VALUE_TYPE,
  FUN_VALUE_TYPE,
} ValueType;

typedef struct Node {
  NodeType type;
  void* data;
} Node;

typedef struct Value {
  ValueType type;
  void* data;
} Value;

typedef struct Env Env;
struct Env {
  HashTable* t;
  List* loopStates;
  jmp_buf retState;
};


Node* newNode(NodeType type, void* data);
Node* newNode2(NodeType type, int n, ...);
void* copy(void* t, int size);

Value* newIntValue(int x);
Value* newFunValue(Node* t);

Env* newEnv();
Value* envGet(Env* e, char* key);
void envPut(Env* e, char* key, Value* value);

void init();
Value* eval(Env* e, Node* p);

void printValue(Value* v);

extern Env* global;

#define YYSTYPE Node*

#endif
