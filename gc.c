#include "gc.h"
#include "list.h"
#include "value.h"
#include "env.h"
#include "closure.h"
#include "util.h"

List* values;
List* rootValues;

static void mark() {
  Value* v;
  List* q = newList();
  int i, n = listSize(rootValues);
  for(i=0; i<n; i++) {
    listPush(q, listGet(rootValues, i));
  }
  while(listSize(q)) {
    v = listPop(q);
    if(v->mark) continue;
    v->mark = 1;
    switch(v->type) {
      case CLOSURE_VALUE_TYPE: {
        Closure* c = v->data;
        listPush(q, c->e);
        break;
      }
      case ENV_VALUE_TYPE: {
        Env* e = v->data;                    
        hashTableAddAllToList(e->t, q);
        listPush(q, e->parent);
        break;
      }
      case LIST_VALUE_TYPE: {
        List* l = v->data;
        n = listSize(l);
        for(i=0;i<n;i++) {
          listPush(q, listGet(l, i));
        }
        break;
      }
      case STRING_VALUE_TYPE:
      case INT_VALUE_TYPE:
      case NONE_VALUE_TYPE: break;
      defalut: error("cannot mark unknown value\n");
    }
  }
  freeList(q);
}

void forceGC() {
  int i, n;
  List* values2 = newList();
  mark();
  n = listSize(values);
  for(i=0;i<n;i++) {
    Value* v = listGet(values, i);
    if(v->mark) {
      v->mark = 0;
      listPush(values2, v);
    } else {
      freeValue(v);
    }
  }
  freeList(values);
  values = values2;
}

extern int hardMemLimit, softMemLimit;

void gc() {
  if(listSize(values) >= hardMemLimit) {
    //fprintf(stderr, "*");
    //fflush(stderr);
    forceGC();
    if(listSize(values) >= softMemLimit) {
      listCreatedObjectsCount();
      error("out of memory\n");
    }
  }
}
