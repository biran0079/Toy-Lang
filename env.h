#ifndef _ENV_H_
#define _ENV_H_

#include "tl.h"
#include "hash_table.h"
#include "list.h"
#include <setjmp.h>

struct Env {
  HashTable* t;
  List* loopStates;
  List* exceptionStates;
  Value* exceptionValue;
  Env* parent;
  jmp_buf retState;
  Value* returnValue;
  // tail call closure with arguments set.  used to pass the tail recursion call node to upper stack frame
  Closure* tailCall;
};

Env* newEnv(Env* parent);
void freeEnv(Env* e);
Value* envGet(Env* e, char* key);
void envPut(Env* e, char* key, Value* value);
void envPutLocal(Env* e, char* key, Value* value);

#endif
