#include "env.h"
#include "hashTable.h"
#include "value.h"
#include "list.h"
#include "util.h"
#include "idMap.h"
#include "closure.h"
#include "ast.h"
#include "gc.h"
#include "mem.h"

int newIntValueC = 0, newStringValueC = 0, newClosureValueC = 0,
    newEnvValueC = 0, newListValueC = 0, newBuiltinFunC = 0;
int freeIntValueC = 0, freeStringValueC = 0, freeClosureValueC = 0,
    freeEnvValueC = 0, freeListValueC = 0, freeBuiltinFunC = 0;

Value *newNoneValue() {
  static Value *res = 0;
  if (res) {
    return res;
  } else {
    res = allocValue();
    res->type = NONE_VALUE_TYPE;
    res->data = 0;
    res->ref = 0;
    res->mark = UNMARKED;
    return res;
  }
}

Value *newIntValue(long x) {
  gc();
  newIntValueC++;
  Value *res = allocValue();
  res->type = INT_VALUE_TYPE;
  res->data = (void *)x;
    res->ref = 0;
  res->mark = UNMARKED;
  return res;
}

Value *newStringValue(char *s) {
  gc();
  newStringValueC++;
  Value *res = allocValue();
  res->type = STRING_VALUE_TYPE;
  res->data = (void *)s;
    res->ref = 0;
  res->mark = UNMARKED;
  return res;
}

Value *newListValue(List *list) {
  gc();
  newListValueC++;
  Value *res = allocValue();
  res->type = LIST_VALUE_TYPE;
  res->data = list;
    res->ref = 0;
  res->mark = UNMARKED;
  int i, n = listSize(list);
  for ( i = 0; i < n;i++) {
    ref(listGet(list, i));
  }
  return res;
}

Value *newClosureValue(Node *t, Env *e) {
  gc();
  newClosureValueC++;
  Value *res = allocValue();
  res->type = CLOSURE_VALUE_TYPE;
  res->data = newClosure(t, e->envValue);
    res->ref = 0;
  res->mark = UNMARKED;
  return res;
}

Value *newBuiltinFun(BuiltinFun f) {
  gc();
  newBuiltinFunC++;
  Value *res = allocValue();
  res->type = BUILTIN_FUN_VALUE_TYPE;
  res->data = f;
    res->ref = 0;
  res->mark = UNMARKED;
  return res;
}

Value *newEnvValue(Env *parent) {
  gc();
  Value *pv = parent ? parent->envValue : newNoneValue();
  newEnvValueC++;
  Value *res = allocValue();
  res->type = ENV_VALUE_TYPE;
  res->data = newEnv(pv, res);
  res->mark = UNMARKED;
    res->ref = 0;
  envPutLocal(res->data, getIntId("this"), res);
  return res;
}

void freeValue(Value *v) {
#ifdef DEBUG_GC
  printf("freeing %s @ %p\n", valueToString(v), v);
#endif
  if (v->type == FREED_TYPE) error("freeing a value twice.\n");
  if (v->type == FREEING_TYPE) return;
  int i, n;
  ValueType t = v->type;
  v->type = FREEING_TYPE;
  switch (t) {
    case CLOSURE_VALUE_TYPE:
      freeClosure((Closure *)v->data);
      freeClosureValueC++;
      break;
    case ENV_VALUE_TYPE:
      freeEnv((Env *)v->data);
      freeEnvValueC++;
      break;
    case LIST_VALUE_TYPE:
      n = listSize(v->data);
      for (i = 0; i < n; i++) {
        deref(listGet(v->data, i));
      }
      freeList(v->data);
      freeListValueC++;
      break;
    case STRING_VALUE_TYPE:
      tlFree((char *)v->data);
      freeStringValueC++;
      break;
    case INT_VALUE_TYPE:
      freeIntValueC++;
      break;
    case BUILTIN_FUN_VALUE_TYPE:
      freeBuiltinFunC++;
      break;
    case NONE_VALUE_TYPE:
      return;  // none is never freed
    case FREED_TYPE:
    case FREEING_TYPE:
      error("never reach here\n");
    default:
      error("unkonwn value type passed to freeValue: %d\n", v->type);
  }
  listPush(deadValues, v);
  v->type = FREED_TYPE;
}

static int valueToStringInternal(Value *v, char *s, int n, HashTable* vis) {
  if (hashTableGet(vis, v)) {
    return mySnprintf(s, n, "...", v->data);
  }
  hashTablePut(vis, v, (void*) 1L);
  Node *t;
  int i, len = 0;
  switch (v->type) {
    case BUILTIN_FUN_VALUE_TYPE:
      len += mySnprintf(s, n, "<builtin-function@%p>", v->data);
      break;
    case NONE_VALUE_TYPE:
      len += mySnprintf(s, n, "none");
      break;
    case INT_VALUE_TYPE:
      len += mySnprintf(s, n, "%d", v->data);
      break;
    case CLOSURE_VALUE_TYPE: {
      Node *f = ((Closure *)v->data)->f;
      len += mySnprintf(s + len, n - len, "fun %s(",
                        getStrId((long)chld(f, 0)->data));
      t = chld(f, 1);
      for (i = 0; i < chldNum(t); i++) {
        if (i) {
          len += mySnprintf(s + len, n - len, ", ");
        }
        len += mySnprintf(s + len, n - len, "%s",
                          getStrId((long)chld(t, i)->data));
      }
      len += mySnprintf(s + len, n - len, ")");
      break;
    }
    case LIST_VALUE_TYPE: {
      List *l = v->data;
      len += mySnprintf(s + len, n - len, "[");
      for (i = 0; i < listSize(l); i++) {
        if (i) {
          len += mySnprintf(s + len, n - len, ", ");
        }
        len += valueToStringInternal(listGet(l, i), s + len, n - len, vis);
      }
      len += mySnprintf(s + len, n - len, "]");
      break;
    }
    case STRING_VALUE_TYPE:
      len += mySnprintf(s, n, "%s", v->data);
      break;
    case ENV_VALUE_TYPE: {
      List *keys = envGetAllIds(v->data);
      len += mySnprintf(s, n, "env parent->%p { ", getEnvFromValue(v)->parent);
      int keyN = listSize(keys);
      for (i = 0; i < keyN; i++) {
        long key = (long)listGet(keys, i);
        len += mySnprintf(s + len, n - len, "%s->%p ", getStrId(key),
                          envGet(v->data, key));
      }
      len += mySnprintf(s + len, n - len, "}");
      freeList(keys);
      break;
    }
    case FREED_TYPE:
      len += mySnprintf(s, n, "<FREED @ %p>", v);
      break;
    case FREEING_TYPE:
      len += mySnprintf(s, n, "<FREEING @ %p>", v);
      break;
    default:
      error("cannot print unknown value type: %d at %p\n", v->type, v);
  }
  hashTableRemove(vis, v);
  return len;
}

char *valueToString(Value *v) {
  HashTable *vis = newIntHashTable();
  int l = valueToStringInternal(v, 0, 0, vis) + 1;
  assert(hashTableSize(vis) == 0);
  char *s = (char *)tlMalloc(l);
  valueToStringInternal(v, s, l, vis);
  freeHashTable(vis);
  return s;
}

int valueEquals(Value *v1, Value *v2) {
  if (v1 == v2) return 1;
  if (v1->type != v2->type) return 0;
  switch (v1->type) {
    case INT_VALUE_TYPE:
    case ENV_VALUE_TYPE:
    case BUILTIN_FUN_VALUE_TYPE:
    case CLOSURE_VALUE_TYPE:
      return v1->data == v2->data;
    case LIST_VALUE_TYPE: {
      List *l1 = v1->data;
      List *l2 = v2->data;
      int len = listSize(l1);
      int i;
      if (len != listSize(l2)) return 0;
      for (i = 0; i < len; i++)
        if (!valueEquals(listGet(l1, i), listGet(l2, i))) return 0;
      return 1;
    }
    case STRING_VALUE_TYPE:
      return strcmp((char *)v1->data, (char *)v2->data) == 0;
    case NONE_VALUE_TYPE:
      return 1;  // singleton, same type means equal
    default:
      error("unknown value type passed to valueEquals: %d\n", v1->type);
  }
  return 0;
}

Value *valueSub(Value *v1, Value *v2) {
  if (v1->type != INT_VALUE_TYPE || v2->type != INT_VALUE_TYPE)
    error("- operator only works for int");
  return newIntValue((long)v1->data - (long)v2->data);
}

Value *valueMul(Value *v1, Value *v2) {
  if (v1->type != INT_VALUE_TYPE || v2->type != INT_VALUE_TYPE)
    error("* operator only works for int");
  return newIntValue((unsigned long)v1->data * (unsigned long)v2->data);
}

Value *valueDiv(Value *v1, Value *v2) {
  if (v1->type != INT_VALUE_TYPE || v2->type != INT_VALUE_TYPE)
    error("/ operator only works for int");
  if (!v2->data) error("denominator cannot be zero\n");
  return newIntValue((long)v1->data / (long)v2->data);
}

Value *valueMod(Value *v1, Value *v2) {
  if (v1->type != INT_VALUE_TYPE || v2->type != INT_VALUE_TYPE)
    error("%% operator only works for int");
  if (!v2->data) error("denominator cannot be zero\n");
  return newIntValue((long)v1->data % (long)v2->data);
}

Value *valueAdd(Value *v1, Value *v2) {
  switch (v1->type) {
    case INT_VALUE_TYPE:
      switch (v2->type) {
        case INT_VALUE_TYPE:
          return newIntValue((long)v1->data + (long)v2->data);
        default:
          error("int can only add to int\n");
      }
    case LIST_VALUE_TYPE: {
      Value *res = newListValue(listCopy(v1->data));
      int i;
      switch (v2->type) {
        case LIST_VALUE_TYPE: {
          for (i = 0; i < listValueSize(v2); i++) {
            listValuePush(res, listValueGet(v2, i));
          }
          break;
        }
        default: {
          listValuePush(res, v2);
          break;
        }
      }
      return res;
    }
    case STRING_VALUE_TYPE: {
      if (v2->type == STRING_VALUE_TYPE) {
        char *s1 = (char *)v1->data, *s2 = (char *)v2->data;
        char *res = (char *)tlMalloc(strlen(s1) + strlen(s2) + 1);
        *res = 0;
        strcat(res, s1);
        strcat(res, s2);
        return newStringValue(res);
      } else {
        error("can only add string to string\n");
      }
    }
    case CLOSURE_VALUE_TYPE:
      error("Closure value cannot add to anything\n");
    case ENV_VALUE_TYPE:
      error("Environment value cannot add to anything\n");
    default:
      error("unknown value type passed to valueAdd: %d\n", v1->type);
  }
  return 0;  // never reach here
}

int valueCmp(Value *v1, Value *v2) {
  if (v1->type != v2->type) {
    return -1;  // comparing different type
  }
  if (v1 == v2) {
    return 0;
  }
  switch (v1->type) {
    case LIST_VALUE_TYPE: {
      List *l1 = v1->data, *l2 = v2->data;
      int i, n1 = listSize(l1), n2 = listSize(l2);
      for (i = 0; i < n1 && i < n2; i++) {
        int res = valueCmp(listGet(l1, i), listGet(l2, i));
        if (res) return res;
      }
      if (n1 == n2)
        return 0;
      else if (n1 < n2)
        return -1;
      else
        return 1;
    }
    case STRING_VALUE_TYPE:
      return strcmp(v1->data, v2->data);
    case CLOSURE_VALUE_TYPE:
    case ENV_VALUE_TYPE:
    case BUILTIN_FUN_VALUE_TYPE:
    case INT_VALUE_TYPE:
      return (long)(v1->data) - (long)(v2->data);
    case NONE_VALUE_TYPE:
      return 0;
    default:
      error("cannot compare unknown type: %d\n", v1->type);
  }
  return 0;
}

long getIntFromValue(Value *intValue) {
  assert(intValue->type == INT_VALUE_TYPE);
  return (long)intValue->data;
}

Env *getEnvFromValue(Value *v) {
  assert(v->type == ENV_VALUE_TYPE);
  return (Env *)v->data;
}

void listValuePush(Value* lv, Value* v) {
  assert(lv->type == LIST_VALUE_TYPE);
  listPush(lv->data, ref(v));
}

void listValueSet(Value* lv, int i, Value* v) {
  assert(lv->type == LIST_VALUE_TYPE);
  deref(listSet(lv->data, i, ref(v)));
}

Value* listValueGet(Value* lv, int i) {
  assert(lv->type == LIST_VALUE_TYPE);
  return listGet(lv->data, i);
}

int listValueSize(Value* lv) {
  assert(lv->type == LIST_VALUE_TYPE);
  return listSize(lv->data);
}

void listValueExtend(Value* lv1, Value* lv2) {
  int i, n = listValueSize(lv2);
  for (i = 0; i < n; i++) {
    listValuePush(lv1, listValueGet(lv2, i));
  }
}

#ifdef DEBUG_GC
char* getEvaluating() {
  if (listSize(astStack) == 0) return copyStr("<empty>");
  return nodeToString(listLast(astStack));
}
#endif
void deref(Value* v) {
#ifdef DEBUG_GC
  char* evaluating = getEvaluating();
  printf("deref (%d) %s @ %p  evaling %s\n", v->ref-1,  valueToString(v), v, evaluating);
  tlFree(evaluating);
#endif
  if (v->ref == 1 && v->mark == MARKED) {
    printf("!!!!\n");
  }
  int ref = --(v->ref); 
  assert(ref >= 0);
  if(!ref) {
    if (v->type != FREEING_TYPE && v->type != FREED_TYPE) {
      freeValue(v);
    }
  }
}

Value* ref(Value* v) {
#ifdef DEBUG_GC
  char* evaluating = getEvaluating();
  printf("ref (%d) %s @ %p  evaling %s\n", v->ref+1,  valueToString(v), v, evaluating);
  tlFree(evaluating);
#endif
  if (v->type == FREED_TYPE) {
    error("refering freed object\n");
  }
  if (v->type == FREEING_TYPE) {
    error("refering freeing object\n");
  }
  v->ref++;
  return v;
}
