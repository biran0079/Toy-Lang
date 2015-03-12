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
List* allValues;

extern int memoryUsage, memoryLimit;

void initMem() {
  allValues = newList();
}

// called after gc
void cleanupMem() {
  freeList(allValues);
}

Value* allocValue() {
  gc();
  Value* v = tlMalloc(sizeof(Value));
  listPush(allValues, v);
  return v;
}

void freeUnmarkedValues() {
  List* newAllValues = newList();
  int i, n = listSize(allValues);
  for (i = 0; i < n; i++) {
    Value* v = listGet(allValues, i);
    if (v->mark == MARKED) {
      v->mark = UNMARKED;
      listPush(newAllValues, v);
    } else {
      assert(v->mark == UNMARKED);
      freeValue(v);
      tlFree(v);
    }
  }
  freeList(allValues);
  allValues = newAllValues;
}

void dumpValueMemory() {
  int i, n = listSize(allValues);
  for (i = 0; i < n; i++) {
    Value* v = listGet(allValues, i);
    printf("    %s @ %p\n", valueToString(v), v);
  }
  printf("\n");
}
