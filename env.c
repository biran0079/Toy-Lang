#include "env.h"
#include "util.h"

extern int newEnvC, freeEnvC;

Env* newEnv(Env* parent){
  newEnvC++;
  Env* res = MALLOC(Env);
  res->t = newHashTable();
  res->parent = parent;
  res->loopStates = newList();
  res->exceptionStates = newList();
  res->exceptionValue = 0;
  res->returnValue = 0;
  res->tailCall = 0;
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
  e->exceptionValue = 0;
  e->returnValue = 0;
  e->tailCall = 0;
  free(e);
}

Value* envGet(Env* e, char* key){
  if(!e)return 0;
  Value* res = (Value*) hashTableGet(e->t, key);
  return res ? res : envGet(e->parent, key);
}

void envPut(Env* e, char* key, Value* value){
  Env* e2 = e;
  while(e2){
    if(hashTableGet(e2->t, key)){
      hashTablePut(e2->t, key, (void*) value);
      return;
    }else{
      e2 = e2->parent;
    }
  }
  hashTablePut(e->t, key, (void*) value);
}

void envPutLocal(Env* e, char* key, Value* value){
  hashTablePut(e->t, key, value);
}

