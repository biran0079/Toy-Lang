#ifndef _EVAL_H_
#define _EVAL_H_
#include "tl.h"

Value* eval(Value* ev, Node* p);
void throwValue(Env* e, Value* v);

#endif
