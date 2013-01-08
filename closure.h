#ifndef _CLOSURE_H_
#define _CLOSURE_H_

#include "tl.h"

struct Closure {
  Node* f;
  Env* e;
};

Closure* newClosure(Node* f, Env* e);
void freeClosure(Closure* c);


#endif
