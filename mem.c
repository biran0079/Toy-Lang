#include "mem.h"
#include "util.h"
#include "closure.h"
#include "env.h"
#include "hashTable.h"
#include "opStack.h"
#include "eval.h"
#include <assert.h>

List *liveValues, *deadValues;

extern int memoryUsage, memoryLimit;

void initValuesBlock() {
  liveValues = newList();
  deadValues = newList();
}

void cleanupValuesBlock() {
  assert(listSize(liveValues) == 0);
  int i, n = listSize(deadValues);
  for (i = 0; i < n; i++) {
    tlFree(listGet(deadValues, i));
  }
  freeList(liveValues);
  freeList(deadValues);
}

Value* allocValue() {
  Value* v;
  if (listSize(deadValues)) {
    v = listPop(deadValues);
  } else {
    v = tlMalloc(sizeof(Value));
  }
  listPush(liveValues, v);
  return v;
}

void freeUnmarkedValues() {

}

int getInMemoryValueCount() {
  return listSize(liveValues);
}

void dumpValueMemory() {
  int i, n = listSize(liveValues);
  for (i = 0; i < n; i++) {
    Value* v = listGet(liveValues, i);
    printf("    %s @ %p\n", valueToString(v), v);
  }
  printf("\n");
}
