#include "env.h"
#include "ast.h"
#include "value.h"
#include "util.h"
#include "eval.h"

int newEnvC = 0, freeEnvC = 0;

Env *newEnv(Value *parentEnv, Value* envValue) {
  newEnvC++;
  Env *res = MALLOC(Env);
  res->t = newIntHashTable();
  res->parent = parentEnv;
  res->envValue = envValue;
  return res;
}

void freeEnv(Env *e) {
  if (!e) error("NONE passed to freeEnv\n");
  freeEnvC++;
  freeHashTable(e->t);
  e->t = 0;
  tlFree(e);
}

Value *envGet(Env *e, long key) {
  while (1) {
    Value *res = (Value *)hashTableGet(e->t, (void *)key);
    if (res) {
      return res;
    }
    if (e->parent->type == NONE_VALUE_TYPE) break;
    e = e->parent->data;
  }
  return newNoneValue();
}

Value* envPut(Env *e, long key, Value *value) {
  Env *e2 = e;
  while (1) {
    if (hashTableGet(e2->t, (void *)key)) {
      return (Value*) hashTablePut(e2->t, (void *)key, (void *)value);
    } else {
      if (e2->parent->type == NONE_VALUE_TYPE) break;
      e2 = e2->parent->data;
    }
  }
  return (Value*) hashTablePut(e->t, (void *)key, (void *)value);
}

Value* envPutLocal(Env *e, long key, Value *value) {
  return (Value*) hashTablePut(e->t, (void *)key, value);
}

List* envGetAllIds(Env* e) {
  return hashTableGetAllKeys(e->t);
}
