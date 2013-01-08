#include"tl.h"

extern int newNodeC, newNode2C, newIntValueC, newStringValueC, newClosureValueC, 
    newListValueC, newClosureC, newEnvC;

Value* newNoneValue(){
  static Value* res = 0;
  if(res) {
    return res;
  } else {
    res = MALLOC(Value);
    res->ref = 0;
    res->type = NONE_VALUE_TYPE;
    res->data = 0;
    return res;
  }
}

Value* newIntValue(long x){
  newIntValueC++;
  Value* res=MALLOC(Value);
  res->ref = 0;
  res->type = INT_VALUE_TYPE;
  res->data = (void*) x;
  return res;
}

Value* newStringValue(char *s){
  newStringValueC++;
  Value* res=MALLOC(Value);
  res->ref = 0;
  res->type = STRING_VALUE_TYPE;
  res->data = (void*) s;
  return res;
}

Value* newListValue(List* list) {
  newListValueC++;
  Value* res = MALLOC(Value);
  res->ref = 0;
  res->type = LIST_VALUE_TYPE;
  res->data = list;
  return res;
}

Value* newClosureValue(Node* t, Env* e) {
  newClosureC++;
  Value* res=MALLOC(Value);
  res->ref = 0;
  res->type = CLOSURE_VALUE_TYPE;
  res->data = newClosure(t, e);
  return res;
}

void valueRefInc(Value* v) {
  v->ref++;
}

void valueRefDec(Value* v) {
  v->ref--;
  if(!v->ref) {
    freeValue(v);
  }
}

void freeValue(Value* v) {
  // TODO
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
    case NONE_VALUE_TYPE:
      return mySnprintf(s, n, "none");
    case INT_TYPE:
      return mySnprintf(s, n, "%d", v->data);
    case CLOSURE_VALUE_TYPE: {
      int len = 0;
      Node* f = ((Closure*) v->data)->f;
      len += mySnprintf(s + len, n - len, "fun %s(", chld(f, 0)->data);
      t = chld(f, 1);
      for(i=0;i<chldNum(t);i++){
        if(i){
          len += mySnprintf(s+len, n-len, ", ");
        }
        len += mySnprintf(s+len, n-len, "%s", chld(t, i)->data);
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
    default: error("cannot print unknown value type\n");
  }
}

char* valueToString(Value* v) {
  int l = valueToStringInternal(v, 0, 0) + 1;
  char *s = (char*) malloc(l);
  valueToStringInternal(v, s, l);
  return s;
}

int valueEquals(Value* v1, Value* v2){
  if(v1 == v2) return 1;
  if(v1->type != v2->type) return 0;
  switch(v1->type){
    case INT_VALUE_TYPE:
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
    case NONE_TYPE: return 1; // singleton, same type means equal
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
        char *res = (char*) malloc(strlen(s1) + strlen(s2) + 1);
        *res = 0;
        strcat(res, s1);
        strcat(res, s2);
        return newStringValue(res);
      } else {
        error("can only add string to string\n");
      }
    }
    case CLOSURE_VALUE_TYPE: error("Function value cannot add to anything\n");
    default: error("unknown value type passed to valueAdd: %d\n", v1->type);
  }
}

