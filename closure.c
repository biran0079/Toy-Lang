#include "closure.h"
#include "util.h"

extern int newClosureC, freeClosureC;

Closure* newClosure(Node* f, Value* e){
  newClosureC++;
  Closure* res = MALLOC(Closure);
  res->f = f;
  res->e = e;
  return res;
}

void freeClosure(Closure* c) {
  if(!c) error("NONE passed to freeClosure\n");
  freeClosureC++;
  c->f = 0;
  c->e = 0;
  tlFree(c);
}

