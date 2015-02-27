#ifndef _ENV_H_
#define _ENV_H_

#include "core.h"
#include "list.h"
#include "hashTable.h"

struct Env {
  HashTable *t;
  Value *parent;
  Value *envValue; // wrapped in value to be managed by GC
};

Env *newEnv(Value *parentEnv, Value* envValue);
void freeEnv(Env *e);
Value *envGet(Env *e, long key);
Value* envPut(Env *e, long key, Value *value);
Value* envPutLocal(Env *e, long key, Value *value);
List* envGetAllIds(Env* e);

#endif
