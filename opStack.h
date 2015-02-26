#ifndef _OP_STACK_H_
#define _OP_STACK_H_

#include "value.h"
#include "list.h"

void cleanupOpStack();
void initOpStack();

void opStackPush(Value *);
Value *opStackPop();
void opStackPopN(int);
Value* opStackPeek(int);

int opStackSize();
void opStackPopTo(int);

void opStackSave();
void opStackRestore();

void opStackAppendValuesTo(List*);

void showOpStack();
#endif
