#ifndef _EVAL_H_
#define _EVAL_H_
#include "tl.h"
#include "builtinFun.h"

Value* eval(Value* ev, Node* p);
Value* evalBuiltInFun(Value* e, Node* exp_lst, BuitinFun f);
void throwValue(Env* e, Value* v);

#endif
