#include "gc.h"
#include "list.h"
#include "value.h"
#include "env.h"
#include "closure.h"
#include "util.h"
#include "core.h"
#include "opStack.h"
#include "mem.h"

extern List *values, *gcHistory;

static void mark() {
  printf("start marking\n");
  Value *v;
  List *q = newList();
  opStackAppendValuesTo(q);
  while (listSize(q)) {
    v = listPop(q);
    if (v->mark != UNMARKED) continue;
    printf("marking %s @%p\n", valueToString(v), v);
    v->mark = MARKED;
    switch (v->type) {
      case CLOSURE_VALUE_TYPE: {
        Closure *c = v->data;
        listPush(q, c->e);
        break;
      }
      case ENV_VALUE_TYPE: {
        Env *e = v->data;
        hashTableAddAllToList(e->t, q);
        listPush(q, e->parent);
        break;
      }
      case LIST_VALUE_TYPE: {
        List *l = v->data;
        int i, n = listSize(l);
        for (i = 0; i < n; i++) {
          listPush(q, listGet(l, i));
        }
        break;
      }
      case STRING_VALUE_TYPE:
      case INT_VALUE_TYPE:
      case BUILTIN_FUN_VALUE_TYPE:
      case NONE_VALUE_TYPE:
        break;
      default:
        error("cannot mark unknown value %d\n", v->type);
    }
  }
  freeList(q);
  printf("finish marking\n");
}

static void pushGCHistory(int before, int after) {
  GCRecord *r = (GCRecord *)MALLOC(GCRecord);
  r->before = before;
  r->after = after;
  listPush(gcHistory, r);
}

void clearGCHistory() {
  int i, n = listSize(gcHistory);
  for (i = 0; i < n; i++) {
    tlFree(listGet(gcHistory, i));
  }
  freeList(gcHistory);
}

void forceGC() {
  int i, n;
  int before = getInMemoryValueCount();
  printf("gc start\n\n");
  dumpValueMemory();
  mark();
  freeUnmarkedValues();
  consolidateMarkedValues();
  int after = getInMemoryValueCount();
  if (shouldDumpGCHistory) {
    pushGCHistory(before, after);
  }
  dumpValueMemory();
  printf("gc done\n\n");
}

extern int memoryUsage, memoryLimit, gcTestMode;

void gc() {
  if (gcTestMode) {
    forceGC();
    return;
  }
  if (memoryUsage >= memoryLimit) {
    forceGC();
    if (memoryUsage >= memoryLimit) {
      listCreatedObjectsCount();
      error("out of memory\n");
    }
  }
}
