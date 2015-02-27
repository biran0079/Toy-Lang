#include "env.h"
#include "ast.h"
#include "value.h"
#include "util.h"
#include "eval.h"

int newEnvC = 0, freeEnvC = 0;

Env *newEnv(Value *parentEnv, Value *envValue) {
  newEnvC++;
  Env *res = MALLOC(Env);
  res->l = newList();
  res->parent = parentEnv;
  res->envValue = envValue;
  return res;
}

static void derefAllValues(Env* e) {
  int i, n = listSize(e->l);
  for (i = 0; i < n; i++) {
    Value *v = listGet(e->l, i);
    if (v) deref(v);
  }
}

void freeEnv(Env *e) {
  if (!e) error("NONE passed to freeEnv\n");
  freeEnvC++;
  derefAllValues(e);
  freeList(e->l);
  e->l = 0;
  tlFree(e);
}

Value *envGet(Env *e, long key) {
  while (1) {
    Value *res = envGetLocal(e, key);
    if (res) {
      return res;
    }
    if (e->parent->type == NONE_VALUE_TYPE) break;
    e = e->parent->data;
  }
  return newNoneValue();
}

Value *envPut(Env *e, long key, Value *value) {
  Env *e2 = e;
  while (1) {
    if (envGetLocal(e2, key)) {
      return envPutLocal(e2, key, value);
    } else {
      if (e2->parent->type == NONE_VALUE_TYPE) break;
      e2 = e2->parent->data;
    }
  }
  return (Value *)envPutLocal(e, key, value);
}

Value *envPutLocal(Env *e, long key, Value *value) {
  ref(value);
  increaseSizeTo(e->l, key + 1);
  Value* oldValue = (Value *)listSet(e->l, key, value);
  if (oldValue) {
    deref(oldValue);
  }
  return oldValue;
}

Value *envGetLocal(Env *e, long key) {
  return e->l->size > key ? listGet(e->l, key) : 0;
}

List *envGetAllIds(Env *e) {
  List *res = newList();
  long i, n = listSize(e->l);
  for (i = 0; i < n; i++) {
    if (listGet(e->l, i)) {
      listPush(res, (void *)i);
    }
  }
  return res;
}

void envAddAllValuesToList(Env *e, List *q) {
  int i, n = listSize(e->l);
  for (i = 0; i < n; i++) {
    Value *v = listGet(e->l, i);
    if (v) listPush(q, v);
  }
}

