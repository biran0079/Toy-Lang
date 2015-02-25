#include "opStack.h"
#include "util.h"

static List *opStack;
static List *opStackState;

void initOpStack() {
  opStack = newList();
  opStackState = newList();
}

void cleanupOpStack() {
  if (listSize(opStack) || listSize(opStackState)) {
    error("cannot clean up op stack when it is not empty");
  }
  freeList(opStack);
  freeList(opStackState);
}

void opStackPush(Value *v) { listPush(opStack, v); }

Value *opStackPop() {
  if (listSize(opStack) == 0) {
    error("cannot pop empty op stack");
  }
  return listPop(opStack);
}

void opStackSave() { listPush(opStackState, (void *)(long) listSize(opStack)); }

void opStackRestore() {
  if (listSize(opStackState) == 0) {
    error("cannot restore without save\n");
  }
  listPopTo(opStack, (int)listPop(opStackState));
}
