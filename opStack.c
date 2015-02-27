#include "opStack.h"
#include "util.h"
#include "list.h"
#include "value.h"

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

void opStackPopN(int n) {
  int newSize = listSize(opStack) - n;
  if (newSize < 0) {
    error("cannot pop empty op stack");
  }
  listPopTo(opStack, newSize);
}

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
  listPopTo(opStack, (long)listPop(opStackState));
}

void opStackAppendValuesTo(List *l) {
  int i, n = listSize(opStack);
  for (i = 0; i < n; i++) listPush(l, listGet(opStack, i));
}

Value *opStackPeek(int i) {
  return listGet(opStack, listSize(opStack) - 1 - i);
}

int opStackSize() { return listSize(opStack); }

void opStackPopTo(int n) { listPopTo(opStack, n); }

void showOpStack() {
  int i, n = listSize(opStack);
  printf("[\n");
  for (i = 0; i < n; i++) {
    printf("%s @ %p\n", valueToString(opStackPeek(i)), opStackPeek(i));
  }
  printf("]\n");
}

void opStackPopToPush(int n, Value *v) {
  if (n == listSize(opStack)) {
    opStackPush(v);
  } else {
    listSet(opStack, n, v);
    listPopTo(opStack, n + 1);
  }
}

void opStackPopNPush(int n, Value *v) {
  opStackPopToPush(listSize(opStack) - n, v);
}

void opStackUpdateAddr(HashTable *addrMap) {
  int i, n = opStackSize();
  for (i = 0; i < n; i++) {
    void *oldAddr = listGet(opStack, i);
    void *newAddr = hashTableGet(addrMap, oldAddr);
    assert(newAddr);
    listSet(opStack, i, newAddr);
  }
}
