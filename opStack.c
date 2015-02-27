#include "opStack.h"
#include "util.h"
#include "list.h"
#include "value.h"

static List *opStack;

void initOpStack() {
  opStack = newList();
}

void cleanupOpStack() {
  if (listSize(opStack)) {
    error("cannot clean up op stack when it is not empty");
  }
  freeList(opStack);
}

void opStackPush(Value *v) {
  listPush(opStack, ref(v));
}

void opStackPopN(int n) {
  int i;
  for (i = 0; i < n; i++) {
    opStackPop();
  }
}

void opStackPop() {
  if (listSize(opStack) == 0) {
    error("cannot pop empty op stack");
  }
  deref(listPop(opStack));
}

void opStackAppendValuesTo(List *l) {
  int i, n = listSize(opStack);
  for (i = 0; i < n; i++) {
    listPush(l, listGet(opStack, i));
  }
}

Value *opStackPeek(int i) {
  return listGet(opStack, listSize(opStack) - 1 - i);
}

int opStackSize() { return listSize(opStack); }

void opStackPopTo(int n) {
  assert(opStackSize() >= n);
  while (opStackSize() > n) {
    opStackPop();
  }
}

void showOpStack() {
  int i, n = listSize(opStack);
  printf("[\n");
  for (i = 0; i < n; i++) {
    printf("%s @ %p\n", valueToString(opStackPeek(i)), opStackPeek(i));
  }
  printf("]\n");
}

void opStackPopToPush(int n, Value *v) {
  ref(v);
  opStackPopTo(v);
  opStackPush(v);
  deref(v);
}

void opStackPopNPush(int n, Value *v) {
  ref(v);
  opStackPopN(n);
  opStackPush(v);
  deref(v);
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
