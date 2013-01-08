#ifndef _CLOSURE_H_
#define _CLOSURE_H_

#include "tl.h"

struct Closure {
  Node* f;
  Value* e;
};

Closure* newClosure(Node* f, Value* e);
void freeClosure(Closure* c);


#endif
