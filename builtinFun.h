#ifndef _BUILTINFUN_H_
#define _BUILTINFUN_H_
#include "tl.h"
#include "env.h"
#include "list.h"

typedef Value* (*BuitinFun)(List* l); 
void registerBuiltinFunctions(Env* e);
#endif
