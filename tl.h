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
  LIST_TYPE,
  LIST_ACCESS_TYPE,
  LIST_ASSIGN_TYPE,
  LEN_TYPE,
  FOR_TYPE,
  ADDEQ_TYPE,
  LIST_ADDEQ_TYPE,
} NodeType;

typedef enum {
  INT_VALUE_TYPE,
  FUN_VALUE_TYPE,
  LIST_VALUE_TYPE,
} ValueType;

typedef struct {
  NodeType type;
  void* data;
} Node;

typedef struct {
  ValueType type;
  void* data;
} Value;

typedef struct Env Env;

typedef struct {
  Node* f;
  Env* e;
} Closure;

struct Env {
  HashTable* t;
  List* loopStates;
  Env* parent;
  jmp_buf retState;
  Value* returnValue;
};

typedef struct {
  Value* v;
} ReturnValue;

ReturnValue* newReturnValue(Value* v);

Node* newNode(NodeType type, void* data);
Node* newNode2(NodeType type, int n, ...);
void* copy(void* t, int size);
long strToLong(char* s);

Value* newIntValue(long x);
Value* newFunValue(Node* t, Env* e);

Env* newEnv(Env* parent);
Value* envGet(Env* e, char* key);
void envPut(Env* e, char* key, Value* value);

Closure* newClosure(Node* f, Env* e);

// pass in a list of values
Value* newListValue(List* lst);

Value* eval(Env* e, Node* p);

void printValue(Value* v);

Node* chld(Node* e, int i);
int chldNum(Node* t);

int valueEquals(Value* v1, Value* v2);
Value* valueAdd(Value* v1, Value* v2);
Value* valueAddEq(Env* e, Node* v1, Node* v2);

void error(char* msg);

#define YYSTYPE Node*

#endif
