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
  CALL_TYPE,
  TAIL_CALL_TYPE,
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
  TIME_TYPE,
  STRING_TYPE, // liter string
  STR_TYPE,    // str() operator
  ORD_TYPE,
  NONE_TYPE,
  FOREACH_TYPE,
} NodeType;

typedef enum {
  INT_VALUE_TYPE,
  FUN_VALUE_TYPE,
  LIST_VALUE_TYPE,
  STRING_VALUE_TYPE,
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
  // tail call closure with arguments set.  used to pass the tail recursion call node to upper stack frame
  Closure* tailCall;
};

Node* newNode(NodeType type, void* data);
Node* newNode2(NodeType type, int n, ...);
void* copy(void* t, int size);
long strToLong(char* s);
char* literalStringToString(char *s);

Value* newIntValue(long x);
Value* newListValue(List* lst);   // pass in a list of values
Value* newFunValue(Node* t, Env* e);
Value* newStringValue(char* s);

Env* newEnv(Env* parent);
void clearEnv();
Value* envGet(Env* e, char* key);
void envPut(Env* e, char* key, Value* value);

Closure* newClosure(Node* f, Env* e);


Value* eval(Env* e, Node* p);

char* valueToString(Value* v);

Node* chld(Node* e, int i);
int chldNum(Node* t);

int valueEquals(Value* v1, Value* v2);
Value* valueAdd(Value* v1, Value* v2);
Value* valueSub(Value* v1, Value* v2);
Value* valueMul(Value* v1, Value* v2);
Value* valueDiv(Value* v1, Value* v2);
Value* valueMod(Value* v1, Value* v2);
Value* valueAddEq(Env* e, Node* v1, Node* v2);

void markTailRecursions(Node* t);
char* nodeTypeToString(NodeType type);
void nodeToDot(FILE* o, Node* t);

void error(char* msg);

#define YYSTYPE Node*

#endif
