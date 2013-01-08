#ifndef _VALUE_H_
#define _VALUE_H_

#include "tl.h"
#include "list.h"

enum ValueType {
  INT_VALUE_TYPE,
  CLOSURE_VALUE_TYPE,
  LIST_VALUE_TYPE,
  STRING_VALUE_TYPE,
  NONE_VALUE_TYPE,
};

struct Value {
  ValueType type;
  void* data;
};

Value* newIntValue(long x);
Value* newListValue(List* lst);   // pass in a list of values
Value* newClosureValue(Node* t, Env* e);
Value* newStringValue(char* s);
Value* newNoneValue();
void freeValue(Value* v);

char* valueToString(Value* v);

int valueEquals(Value* v1, Value* v2);
Value* valueAdd(Value* v1, Value* v2);
Value* valueSub(Value* v1, Value* v2);
Value* valueMul(Value* v1, Value* v2);
Value* valueDiv(Value* v1, Value* v2);
Value* valueMod(Value* v1, Value* v2);
Value* valueAddEq(Env* e, Node* v1, Node* v2);

#endif