#include "opStack.h"

void opStackInit() {
  opStack = newList();
  opStackState = newList();
}

void opStackCleanup() {}

void opStackPush(Value *);

Value *opStackPop();

void opStackSave();

void opStackRestore();
