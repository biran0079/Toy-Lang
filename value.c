#include "tl.h"
#include "env.h"
#include "value.h"
#include "list.h"
#include "util.h"
#include "closure.h"
#include "ast.h"
#include "gc.h"

extern int newIntValueC, newStringValueC, newClosureValueC, newEnvValueC, newListValueC, newBuiltinFunC;
extern int freeIntValueC, freeStringValueC, freeClosureValueC, freeEnvValueC, freeListValueC, freeBuiltinFunC;
extern List* values;

Value* newNoneValue(){
  static Value* res = 0;
  if(res) {
    return res;
  } else {
    res = MALLOC(Value);
    res->type = NONE_VALUE_TYPE;
    res->data = 0;
    res->mark = 0;
    return res;
  }
}

Value* newIntValue(long x){
  gc();
  newIntValueC++;
  Value* res=MALLOC(Value);
  res->type = INT_VALUE_TYPE;
  res->data = (void*) x;
  res->mark = 0;
  listPush(values, res);
  return res;
}

Value* newStringValue(char *s){
  gc();
  newStringValueC++;
  Value* res=MALLOC(Value);
  res->type = STRING_VALUE_TYPE;
  res->data = (void*) s;
  res->mark = 0;
  listPush(values, res);
  return res;
}

Value* newListValue(List* list) {
  gc();
  newListValueC++;
  Value* res = MALLOC(Value);
  res->type = LIST_VALUE_TYPE;
  res->data = list;
  res->mark = 0;
  listPush(values, res);
  return res;
}

Value* newClosureValue(Node* t, Value* e) {
  gc();
  newClosureValueC++;
  Value* res = MALLOC(Value);
  res->type = CLOSURE_VALUE_TYPE;
  res->data = newClosure(t, e);
  res->mark = 0;
  listPush(values, res);
  return res;
}

Value* newBuiltinFun(BuiltinFun f) {
  gc();
  newBuiltinFunC++;
  Value* res = MALLOC(Value);
  res->type = BUILTIN_FUN_VALUE_TYPE;
  res->data = f;
  res->mark = 0;
  listPush(values, res);
  return res;
}


Value* newEnvValue(Env* e) {
  gc();
  newEnvValueC++;
  Value* res = MALLOC(Value);
  res->type = ENV_VALUE_TYPE;
  res->data = e;
  res->mark = 0;
  listPush(values, res);
  envPutLocal(e, getIntId("this"), res);
  return res;
}

void freeValue(Value* v) {
  switch(v->type) {
    case CLOSURE_VALUE_TYPE:
      freeClosure((Closure*) v->data);
      freeClosureValueC++;
      break;
    case ENV_VALUE_TYPE:
      freeEnv((Env*) v->data);
      freeEnvValueC++;
      break;
    case LIST_VALUE_TYPE:
      freeList((List*) v->data);
      freeListValueC++;
      break;
    case STRING_VALUE_TYPE:
      tlFree((char*) v->data);
      freeStringValueC++;
      break;
    case INT_VALUE_TYPE: {
      freeIntValueC++;
      break;
    }
    case BUILTIN_FUN_VALUE_TYPE: freeBuiltinFunC++; break;
    case NONE_VALUE_TYPE: return; // none is never freed
    default: error("unkonwn value type passed to freeValue: %d\n", v->type);
  }
  v->type = NONE_VALUE_TYPE;
  tlFree(v);
}


static int getStringLength(char *format, ...) {
  va_list ap;
  int len = 0;
  va_start(ap, format);
  len = vsnprintf(0, 0, format, ap);
  va_end(ap);
  return len;
}

static int mySnprintf(char *s, int n, char *format, ...){
  va_list ap;
  va_start(ap, format);
  if(n > 0) {
    return vsnprintf(s, n, format, ap);
  } else {
    return vsnprintf(0, 0, format, ap);
  }
  va_end(ap);
}

static int valueToStringInternal(Value* v, char *s, int n) {
  Node* t;
  int i;
  switch(v->type) {
    case BUILTIN_FUN_VALUE_TYPE:
      return mySnprintf(s, n, "<builtin-function@%p>", v->data);
    case NONE_VALUE_TYPE:
      return mySnprintf(s, n, "none");
    case INT_VALUE_TYPE:
      return mySnprintf(s, n, "%d", v->data);
    case CLOSURE_VALUE_TYPE: {
      int len = 0;
      Node* f = ((Closure*) v->data)->f;
      len += mySnprintf(s + len, n - len, "fun %s(", getStrId((long) chld(f, 0)->data));
      t = chld(f, 1);
      for(i=0;i<chldNum(t);i++){
        if(i){
          len += mySnprintf(s+len, n-len, ", ");
        }
        len += mySnprintf(s+len, n-len, "%s", getStrId((long) chld(t, i)->data));
      }
      len += mySnprintf(s+len, n-len, ")");
      return len;
    }
    case LIST_VALUE_TYPE: {
      int len=0;
      List* l = v->data;
      len += mySnprintf(s+len, n-len, "[");
      for(i=0; i<listSize(l);i++){
        if(i){
          len += mySnprintf(s+len, n-len, ", ");
        }
        len += valueToStringInternal(listGet(l, i), s+len, n-len);
      }
      len += mySnprintf(s+len, n-len, "]");
      return len;
    }
    case STRING_VALUE_TYPE: {
      return mySnprintf(s, n, "%s", v->data);
    }
    case ENV_VALUE_TYPE: {
      return mySnprintf(s, n, "env %p", v->data);                    
    }
    default: error("cannot print unknown value type: %d at %p\n", v->type, v);
  }
}

char* valueToString(Value* v) {
  int l = valueToStringInternal(v, 0, 0) + 1;
  char *s = (char*) tlMalloc(l);
  valueToStringInternal(v, s, l);
  return s;
}

int valueEquals(Value* v1, Value* v2){
  if(v1 == v2) return 1;
  if(v1->type != v2->type) return 0;
  switch(v1->type){
    case INT_VALUE_TYPE:
    case ENV_VALUE_TYPE:
    case BUILTIN_FUN_VALUE_TYPE:
    case CLOSURE_VALUE_TYPE: return v1->data == v2->data;
    case LIST_VALUE_TYPE: {
      List* l1 = v1->data;
      List* l2 = v2->data;
      int len = listSize(l1);
      int i;
      if(len!=listSize(l2))return 0;
      for(i=0;i<len;i++)
        if(!valueEquals(listGet(l1, i), listGet(l2, i)))
          return 0;
      return 1;
    }
    case STRING_VALUE_TYPE: 
      return strcmp((char*) v1->data, (char*) v2->data) == 0;
    case NONE_VALUE_TYPE: return 1; // singleton, same type means equal
    default: error("unknown value type passed to valueEquals: %d\n", v1->type);
  }
}

Value* valueSub(Value* v1, Value* v2) {
  if(v1->type != INT_VALUE_TYPE || v2->type != INT_VALUE_TYPE)
    error("- operator only works for int");
  return newIntValue((long) v1->data - (long) v2->data);
}

Value* valueMul(Value* v1, Value* v2) {
  if(v1->type != INT_VALUE_TYPE || v2->type != INT_VALUE_TYPE)
    error("* operator only works for int");
  return newIntValue((long) v1->data * (long) v2->data);
}

Value* valueDiv(Value* v1, Value* v2) {
  if(v1->type != INT_VALUE_TYPE || v2->type != INT_VALUE_TYPE)
    error("/ operator only works for int");
  if(!v2->data) error("denominator cannot be zero\n");
  return newIntValue((long) v1->data / (long) v2->data);
}

Value* valueMod(Value* v1, Value* v2) {
  if(v1->type != INT_VALUE_TYPE || v2->type != INT_VALUE_TYPE)
    error("%% operator only works for int");
  if(!v2->data) error("denominator cannot be zero\n");
  return newIntValue((long) v1->data % (long) v2->data);
}


Value* valueAdd(Value* v1, Value* v2) {
  switch(v1->type){
    case INT_VALUE_TYPE:
      switch(v2->type){
        case INT_VALUE_TYPE: 
          return newIntValue((long) v1->data + (long) v2->data);
        default: error("int can only add to int\n");
      }
    case LIST_VALUE_TYPE:{
      List* res = listCopy(v1->data);
      int i;
      switch(v2->type){
        case LIST_VALUE_TYPE: {
          List* l = v2->data;
          for(i=0;i<listSize(l);i++)
            listPush(res, listGet(l, i));
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
      if(v2->type == STRING_VALUE_TYPE) {
        char *s1 = (char*) v1->data, *s2 = (char*) v2->data;
        char *res = (char*) tlMalloc(strlen(s1) + strlen(s2) + 1);
        *res = 0;
        strcat(res, s1);
        strcat(res, s2);
        return newStringValue(res);
      } else {
        error("can only add string to string\n");
      }
    }
    case CLOSURE_VALUE_TYPE: error("Closure value cannot add to anything\n");
    case ENV_VALUE_TYPE: error("Environment value cannot add to anything\n");
    default: error("unknown value type passed to valueAdd: %d\n", v1->type);
  }
}

int valueCmp(Value* v1, Value* v2) {
  if(v1->type != v2->type) {
    return -1; // comparing different type
  }
  if(v1 == v2) {
    return 0;
  }
  switch(v1->type) {
    case LIST_VALUE_TYPE: {
      List* l1 = v1->data, *l2 = v2->data;
      int i, n1 = listSize(l1), n2 = listSize(l2);
      for(i=0;i<n1 && i<n2;i++){
        int res = valueCmp(listGet(l1, i), listGet(l2, i));
        if(res)return res;
      }
      if(n1==n2) return 0;
      else if(n1 < n2) return -1;
      else return 1;
    }
    case STRING_VALUE_TYPE: return strcmp(v1->data, v2->data);
    case CLOSURE_VALUE_TYPE: 
    case ENV_VALUE_TYPE:
    case BUILTIN_FUN_VALUE_TYPE: 
    case INT_VALUE_TYPE: return (long)(v1->data) - (long)(v2->data);
    case NONE_VALUE_TYPE: return 0;
    default: error("cannot compare unknown type: %d\n", v1->type);
  }
}
