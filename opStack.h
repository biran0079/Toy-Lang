#ifndef _OP_STACK_H_
#define _OP_STACK_H_

#include "value.h"
#include "list.h"

static List *opStack;
static List *opStackState;

void opStackInit();
void opStackCleanup();

void opStackPush(Value *);
Value *opStackPop();

void opStackSave();
void opStackRestore();

#endif
