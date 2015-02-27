#include "gc.h"
#include "list.h"
#include "value.h"
#include "env.h"
#include "tljmp.h"
#include "closure.h"
#include "util.h"
#include "core.h"
#include "opStack.h"

extern List *values, *rootValues, *gcHistory;

static void mark() {
  Value *v;
  List *q = newList();
  int i, n = listSize(rootValues);
  for (i = 0; i < n; i++) {
    listPush(q, listGet(rootValues, i));
  }
  opStackAppendValuesTo(q);
  if (__jmpMsg__.data) {
    listPush(q, __jmpMsg__.data);
  }
  while (listSize(q)) {
    v = listPop(q);
    if (v->mark) continue;
    v->mark = 1;
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
        for (i = 0; i < envNumOfExceptionStates(e); i++) {
          Exception *ex = envGetExceptionStates(e, i);
          if (ex->finally) mark(ex->finally->ev);
        }
        break;
      }
      case LIST_VALUE_TYPE: {
        List *l = v->data;
        n = listSize(l);
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
        error("cannot mark unknown value\n");
    }
  }
  freeList(q);
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
  int before = listSize(values);
  List *values2 = newList();
  mark();
  n = listSize(values);
  for (i = 0; i < n; i++) {
    Value *v = listGet(values, i);
    if (v->mark) {
      v->mark = 0;
      listPush(values2, v);
    } else {
      freeValue(v);
    }
  }
  freeList(values);
  values = values2;
  int after = listSize(values);
  pushGCHistory(before, after);
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
