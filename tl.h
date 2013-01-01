#ifndef _TL_H_
#define _TL_H_

#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<assert.h>
#include"hash_table.h"
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
} NodeType;

typedef enum {
  INT_VALUE_TYPE,
} ValueType;

typedef struct Node {
  NodeType type;
  void* data;
} Node;

typedef struct Value {
  ValueType type;
  void* data;
} Value;

typedef struct Env {
  HashTable* t;
} Env;


Node* newNode(NodeType type, void* data);
Node* newNode2(NodeType type, int n, ...);
void* copy(void* t, int size);

Value* newIntValue(int x);

Env* newEnv();
Value* envGet(Env* e, char* key);
void envPut(Env* e, char* key, Value* value);

Value* eval(Env* e, Node* p);

void printValue(Value* v);

#define YYSTYPE Node*

#endif
