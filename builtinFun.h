#ifndef _BUILTINFUN_H_
#define _BUILTINFUN_H_
#include "core.h"
#include "list.h"

#ifndef USE_LEGACY_EVAL
typedef void (*BuiltinFun)(int);
#else
typedef Value *(*BuiltinFun)(List *l);
#endif

void registerBuiltinFunctions(Env *e);

#endif
