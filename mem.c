#include "mem.h"
#include "util.h"
#include "closure.h"
#include "env.h"
#include "hashTable.h"
#include "opStack.h"
#include "eval.h"
#include <assert.h>

List *allValues, *deadValues;

extern int memoryUsage, memoryLimit;

void initValuesBlock() {
  allValues = newList();
  deadValues = newList();
}

void cleanupValuesBlock() {
  int i, n = listSize(allValues);
  for (i = 0; i < n; i++) {
    Value* v = listGet(allValues, i);
    if (v->type != FREED_TYPE) {
      error("value is still alive!\n%s\n", valueToString(v));
    }
  }
  freeList(allValues);
  n = listSize(deadValues);
  for (i = 0; i < n; i++) {
    tlFree(listGet(deadValues, i));
  }
  freeList(deadValues);
}

Value* allocValue() {
  Value* v;
  if (listSize(deadValues)) {
    v = listPop(deadValues);
  } else {
    v = tlMalloc(sizeof(Value));
  }
  listPush(allValues, v);
  return v;
}

void freeUnmarkedValues() {
  List* newAllValues = newList();
  int i, n = listSize(allValues);
  for (i = 0; i < n; i++) {
    Value* v = listGet(allValues, i);
    if (v->mark == MARKED) {
      assert(v->type != FREED_TYPE);
      listPush(newAllValues, v);
    } else if (v->type != FREED_TYPE && v->mark == UNMARKED) {
      tlFree(v);
    } 
  }
  freeList(allValues);
  allValues = newAllValues;
}

int getInMemoryValueCount() {
  return listSize(allValues);
}

void dumpValueMemory() {
  int i, n = listSize(allValues);
  for (i = 0; i < n; i++) {
    Value* v = listGet(allValues, i);
    printf("    %s @ %p\n", valueToString(v), v);
  }
  printf("\n");
}
