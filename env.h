#ifndef _ENV_H_
#define _ENV_H_

#include "tl.h"
#include "hashTable.h"
#include "value.h"
#include "list.h"
#include <setjmp.h>

struct Env {
  HashTable* t;
  List* loopStates;
  List* exceptionStates;
  Value* parent;
  jmp_buf retState;
};

Env* newEnv(Value* parentEnv);
void freeEnv(Env* e);
Value* envGet(Env* e, char* key);
void envPut(Env* e, char* key, Value* value);
void envPutLocal(Env* e, char* key, Value* value);

#endif
