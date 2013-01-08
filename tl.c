#include"tl.h"

int newNodeC = 0, newNode2C = 0, newIntValueC = 0, newStringValueC = 0, newClosureValueC = 0, 
    newListValueC = 0, newClosureC = 0, newEnvC = 0;
int freeNodeC = 0, freeNode2C = 0, freeIntValueC = 0, freeStringValueC = 0, freeClosureValueC = 0, 
    freeListValueC = 0, freeClosureC = 0, freeEnvC=0;
int newListC = 0, newHashTableC = 0, freeListC = 0, freeHashTableC = 0;

void listCreatedObjectsCount(){
  fprintf(stderr, "\tNode: %d %d\n", newNodeC, freeNodeC);
  fprintf(stderr, "\tNode2: %d %d\n", newNode2C, freeNode2C);
  fprintf(stderr, "\tIntValue: %d %d\n", newIntValueC, freeIntValueC);
  fprintf(stderr, "\tStringValue: %d %d\n", newStringValueC, freeStringValueC);
  fprintf(stderr, "\tFunValue: %d %d\n", newClosureValueC, freeClosureValueC);
  fprintf(stderr, "\tListValue: %d %d\n", newListValueC, freeListValueC);
  fprintf(stderr, "\tClosure: %d %d\n", newClosureC, freeClosureC);
  fprintf(stderr, "\tEnv: %d %d\n", newEnvC, freeEnvC);
  fprintf(stderr, "\tList: %d %d\n", newListC, freeListC);
  fprintf(stderr, "\tHashTable: %d %d\n", newHashTableC, freeHashTableC);
}

Closure* newClosure(Node* f, Env* e){
  newClosureC++;
  Closure* res = MALLOC(Closure);
  res->ref = 0;
  res->f = f;
  res->e = e;
  envRefInc(e);
  res->ref = 0;
  return res;
}

void closureRefInc(Closure* c) {
  if(!c) error("NONE passed to closureRefInc\n");
  c->ref++;
}

void closureRefDec(Closure* c) {
  if(!c) error("NONE passed to closureRefDec\n");
  c->ref--;
  if(!c->ref) {
    //freeClosure(c);
  }
}

void freeClosure(Closure* c) {
  if(!c) error("NONE passed to freeClosure\n");
  envRefDec(c->e);
  c->f = 0;
  c->e = 0;
}

Env* newEnv(Env* parent){
  newEnvC++;
  Env* res = MALLOC(Env);
  res->ref = 0;
  res->t = newHashTable();
  res->parent = parent;
  res->loopStates = newList();
  res->exceptionStates = newList();
  res->exceptionValue = 0;
  res->returnValue = 0;
  res->tailCall = 0;
  return res;
}

void envRefInc(Env* e){
  if(!e) error("NONE pointer pass to envRefInc\n");
  e->ref++;
}

void envRefDec(Env* e){
  if(!e) error("NONE pointer pass to envRefDec\n");
  e->ref--;
  if(!e->ref) {
    //freeEnv(e);
  }
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
  if(e->exceptionValue) {
    valueRefDec(e->exceptionValue);
    e->exceptionValue = 0;
  }
  if(e->returnValue) {
    valueRefDec(e->returnValue);
    e->returnValue = 0;
  }
  if(e->tailCall) {
    closureRefDec(e->tailCall);
    e->tailCall = 0;
  }
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

void throwValue(Env* e, Value* v) {
  e->exceptionValue = v;  // pass exception value through enviconment
  if(listSize(e->exceptionStates)) {
    // currently in try block
    longjmp(listLast(e->exceptionStates), 1);
  } else {
    // If has parent stack frame, pass to it. Report error otherwise
    if (e->parent) {
      longjmp(e->retState, 3);
    } else {
      error("uncaught exception:\n%s\n", valueToString(v));
    }
  }
  error("should never reach here\n");
}

