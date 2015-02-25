#ifndef _ENV_H_
#define _ENV_H_

#include "core.h"
#include "hashTable.h"
#include "exception.h"
#include "execUnit.h"
#include <setjmp.h>

typedef struct RuntimeState {
  List *loopStates;
  List *exceptionStates;
} RuntimeState;

struct Env {
  HashTable *t;
  Value *parent;
  RuntimeState *state;
  jmp_buf retState;
};

Env *newEnv(Value *parentEnv);
void freeEnv(Env *e);
Value *envGet(Env *e, long key);
void envPut(Env *e, long key, Value *value);
void envPutLocal(Env *e, long key, Value *value);
List *envGetLoopStates(Env *e);
void envPushExceptionStates(Env *e, Exception *ex);
void envPopExceptionStates(Env *e);
Exception *envLastExceptionState(Env *e);
int envNumOfExceptionStates(Env *e);
int envNumOfLoopStates(Env *e);
void envRestoreStates(Env *e, int loopStateNum, int exceptionStateNum);
Exception *envGetExceptionStates(Env *e, int idx);

#endif
