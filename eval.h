#ifndef _EVAL_H_
#define _EVAL_H_
#include "tl.h"
#include "builtinFun.h"

Value* eval(Value* ev, Node* p);
void throwValue(Env* e, Value* v);
void pushRootValue(Value* v);

#endif
