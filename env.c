#include "env.h"
#include "util.h"

extern int newEnvC, freeEnvC;

Env* newEnv(Value* parentEnv){
  newEnvC++;
  Env* res = MALLOC(Env);
  res->t = newHashTable();
  res->parent = parentEnv;
  res->loopStates = newList();
  res->exceptionStates = newList();
  return res;
}

void freeEnv(Env* e) {
  if(!e) error("NONE passed to freeEnv\n");
  freeEnvC++;
  freeHashTable(e->t);
  e->t = 0;
  freeList(e->loopStates);
  freeList(e->exceptionStates);
  e->loopStates = 0;
  e->exceptionStates = 0;
  free(e);
}

Value* envGet(Env* e, char* key){
  while(1) {
    Value* res = (Value*) hashTableGet(e->t, key);
    if(res) {
      return res;
    }
    if(e->parent->type == NONE_VALUE_TYPE)
      break;
    e = e->parent->data;
  }
  return 0;
}

void envPut(Env* e, char* key, Value* value){
  Env* e2 = e;
  while(1){
    if(hashTableGet(e2->t, key)){
      hashTablePut(e2->t, key, (void*) value);
      return;
    }else{
      if(e2->parent->type == NONE_VALUE_TYPE) break;
      e2 = e2->parent->data;
    }
  }
  hashTablePut(e->t, key, (void*) value);
}

void envPutLocal(Env* e, char* key, Value* value){
  hashTablePut(e->t, key, value);
}

