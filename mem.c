#include "mem.h"
#include "util.h"
#include "closure.h"
#include "env.h"
#include "hashTable.h"
#include "opStack.h"
#include "eval.h"
#include <assert.h>

// Make sure no duplicate value in allValues list.
List* allValues, *deadValues;

extern int memoryUsage, memoryLimit;

void initMem() {
  allValues = newList();
  deadValues = newList();
}

// called after gc
void cleanupMem() {
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
    listPush(allValues, v);
  }
  return v;
}

void freeUnmarkedValues() {
  List* newAllValues = newList();
  int i, n = listSize(allValues);
  for (i = 0; i < n; i++) {
    Value* v = listGet(allValues, i);
    if (v->mark == MARKED) {
      assert(v->type != FREED_TYPE);
      v->mark = UNMARKED;
      listPush(newAllValues, v);
    } else if (v->type != FREED_TYPE && v->mark == UNMARKED) {
      freeValue(v);
    }
  }
  freeList(allValues);
  allValues = newAllValues;
  n = listSize(deadValues);
  for (i = 0; i < n; i++) {
    tlFree(listGet(deadValues, i));
  }
  listClear(deadValues);
}

int getInMemoryValueCount() { return listSize(allValues); }

void dumpValueMemory() {
  int i, n = listSize(allValues);
  for (i = 0; i < n; i++) {
    Value* v = listGet(allValues, i);
    printf("    %s @ %p\n", valueToString(v), v);
  }
  printf("\n");
}
