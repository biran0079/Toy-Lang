#include "gc.h"
#include "list.h"
#include "value.h"
#include "env.h"
#include "closure.h"
#include "util.h"
#include "core.h"
#include "opStack.h"
#include "mem.h"

static void mark() {
  Value *v;
  List *q = newList();
  opStackAppendValuesTo(q);
  while (listSize(q)) {
    v = listPop(q);
    if (v->mark != UNMARKED) continue;
#ifdef DEBUG_GC
    printf("marking %s @%p\n", valueToString(v), v);
#endif
    v->mark = MARKED;
    switch (v->type) {
      case CLOSURE_VALUE_TYPE: {
        Closure *c = v->data;
        listPush(q, c->e);
        break;
      }
      case ENV_VALUE_TYPE: {
        Env *e = v->data;
        envAddAllValuesToList(e, q);
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
}

void forceGC() {
#ifdef DEBUG_GC
  printf("gc start\n\n");
  showOpStack();
  dumpValueMemory();
#endif
  mark();
  freeUnmarkedValues();
  consolidateMarkedValues();
#ifdef DEBUG_GC
  dumpValueMemory();
  showOpStack();
  printf("gc done\n\n");
#endif
}

extern int memoryUsage, memoryLimit;

void gc() {
  if (isInitialized && memoryUsage >= memoryLimit) {
    forceGC();
    if (memoryUsage >= memoryLimit) {
      listCreatedObjectsCount();
      error("out of memory\n");
    }
  }
}
