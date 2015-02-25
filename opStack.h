#ifndef _OP_STACK_H_
#define _OP_STACK_H_

#include "value.h"
#include "list.h"

void cleanupOpStack();
void initOpStack();

void opStackPush(Value *);
Value *opStackPop();

void opStackSave();
void opStackRestore();

#endif
