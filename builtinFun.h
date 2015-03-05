#ifndef _BUILTINFUN_H_
#define _BUILTINFUN_H_
#include "core.h"
#include "list.h"
#include "eval.h"

typedef EvalResult* (*BuiltinFun)(int n);

void registerBuiltinFunctions(Env* e);

#endif
