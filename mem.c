#include "mem.h"
#include "gc.h"
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
    gc();
    v = tlMalloc(sizeof(Value));
    listPush(allValues, v);
  }
  return v;
}

void freeUnmarkedValues() {
  List* newAllValues = newList();
  int i, n = listSize(allValues);
  printf("before %d\n", listSize(allValues));
  for (i = 0; i < n; i++) {
    Value* v = listGet(allValues, i);
    if (v->mark == MARKED) {
      v->mark = UNMARKED;
      listPush(newAllValues, v);
    } else {
      assert(v->mark == UNMARKED);
      freeValue(v);
    }
  }
  freeList(allValues);
  allValues = newAllValues;
  printf("after %d\n", listSize(allValues));
  n = listSize(deadValues);
  for (i = 0; i < n; i++) {
    tlFree(listGet(deadValues, i));
  }
  listClear(deadValues);
}

void dumpValueMemory() {
  int i, n = listSize(allValues);
  for (i = 0; i < n; i++) {
    Value* v = listGet(allValues, i);
    printf("    %s @ %p\n", valueToString(v), v);
  }
  printf("\n");
}
