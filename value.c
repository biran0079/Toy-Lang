#include "env.h"
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
    res->mark = STATIC;
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
  int i, n;
  switch (v->type) {
    case CLOSURE_VALUE_TYPE:
      freeClosure((Closure *)v->data);
      freeClosureValueC++;
      break;
    case ENV_VALUE_TYPE:
      freeEnv((Env *)v->data);
      freeEnvValueC++;
      break;
    case LIST_VALUE_TYPE:
      List* l = (List *)v->data;
      n = listSize(l);
      for (i = 0; i < n; i++) deref(listGet(l, i));
      freeList(l);
      freeListValueC++;
      break;
    case STRING_VALUE_TYPE:
      tlFree((char *)v->data);
      freeStringValueC++;
      break;
    case INT_VALUE_TYPE: {
      freeIntValueC++;
      break;
    }
    case BUILTIN_FUN_VALUE_TYPE:
      freeBuiltinFunC++;
      break;
    case NONE_VALUE_TYPE:
      return;  // none is never freed
    default:
      error("unkonwn value type passed to freeValue: %d\n", v->type);
  }
  v->type = NONE_VALUE_TYPE;
}

static int getStringLength(char *format, ...) {
  va_list ap;
  int len = 0;
  va_start(ap, format);
  len = vsnprintf(0, 0, format, ap);
  va_end(ap);
  return len;
}

static int mySnprintf(char *s, int n, char *format, ...) {
  va_list ap;
  va_start(ap, format);
  if (n > 0) {
    return vsnprintf(s, n, format, ap);
  } else {
    return vsnprintf(0, 0, format, ap);
  }
  va_end(ap);
}

static int valueToStringInternal(Value *v, char *s, int n) {
  Node *t;
  int i;
  switch (v->type) {
    case BUILTIN_FUN_VALUE_TYPE:
      return mySnprintf(s, n, "<builtin-function@%p>", v->data);
    case NONE_VALUE_TYPE:
      return mySnprintf(s, n, "none");
    case INT_VALUE_TYPE:
      return mySnprintf(s, n, "%d", v->data);
    case CLOSURE_VALUE_TYPE: {
      int len = 0;
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
      return len;
    }
    case LIST_VALUE_TYPE: {
      int len = 0;
      List *l = v->data;
      len += mySnprintf(s + len, n - len, "[");
      for (i = 0; i < listSize(l); i++) {
        if (i) {
          len += mySnprintf(s + len, n - len, ", ");
        }
        len += valueToStringInternal(listGet(l, i), s + len, n - len);
      }
      len += mySnprintf(s + len, n - len, "]");
      return len;
    }
    case STRING_VALUE_TYPE: {
      return mySnprintf(s, n, "%s", v->data);
    }
    case ENV_VALUE_TYPE: {
      List *keys = envGetAllIds(v->data);
      int len = 0;
      len += mySnprintf(s, n, "env parent->%p { ", getEnvFromValue(v)->parent);
      int keyN = listSize(keys);
      for (i = 0; i < keyN; i++) {
        long key = (long)listGet(keys, i);
        len += mySnprintf(s + len, n - len, "%s->%p ", getStrId(key),
                          envGet(v->data, key));
      }
      len += mySnprintf(s + len, n - len, "}");
      freeList(keys);
      return len;
    }
    default:
      error("cannot print unknown value type: %d at %p\n", v->type, v);
  }
}

char *valueToString(Value *v) {
  int l = valueToStringInternal(v, 0, 0) + 1;
  char *s = (char *)tlMalloc(l);
  valueToStringInternal(v, s, l);
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
      List *res = listCopy(v1->data);
      int i;
      switch (v2->type) {
        case LIST_VALUE_TYPE: {
          List *l = v2->data;
          for (i = 0; i < listSize(l); i++) listPush(res, listGet(l, i));
          break;
        }
        default: {
          listPush(res, v2);
          break;
        }
      }
      return newListValue(res);
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
}

long getIntFromValue(Value *intValue) {
  assert(intValue->type == INT_VALUE_TYPE);
  return (long)intValue->data;
}

Env *getEnvFromValue(Value *v) {
  assert(v->type == ENV_VALUE_TYPE);
  return (Env *)v->data;
}

Value* ref(Value* v) {
  v->ref++;
  return v;
}

void listValuePush(Value* lv, Value* v) {
  assert(lv->type == LIST_VALUE_TYPE);
  listPush(lv->data, ref(v));
}

void listValueSet(Value* lv, int i, Value* v) {
  assert(lv->type == LIST_VALUE_TYPE);
  deref(listSet(lv->data, i, ref(v)));
}

void listValueGet(Value* lv, int i) {
  assert(lv->type == LIST_VALUE_TYPE);
  return listGet(l->data, i);
}

int listValueSize(Value* lv) {
  assert(lv->type == LIST_VALUE_TYPE);
  return listSize(lv->data);
}

void listValueExtend(Value* lv1, Value* lv2) {
  int i, n = listValueSize(lv2);
  for (int i = 0; i < n; i++) {
    listValuePush(lv1, listValueGet(lv2, i));
  }
}

void deref(Value* v) {
  int ref = --(v->ref); 
  assert(ref >= 0);
  if(!ref) {
    freeValue(v);
  }
}
