#ifndef _VALUE_H_
#define _VALUE_H_

#include "core.h"
#include "builtinFun.h"

enum ValueType {
  INT_VALUE_TYPE,
  CLOSURE_VALUE_TYPE,
  ENV_VALUE_TYPE,
  LIST_VALUE_TYPE,
  STRING_VALUE_TYPE,
  BUILTIN_FUN_VALUE_TYPE,
  NONE_VALUE_TYPE,
};

typedef enum Mark {
  UNMARKED = 1,
  MARKED = 2,
  STATIC = 4,
} Mark;

struct Value {
  ValueType type;
  Mark mark;
  void *data;
};

Value *newIntValue(long x);
Value *newListValue(List *lst);  // pass in a list of values
// need to pass pointer of value pointer to make sure the value is up to date after gc 
Value *newClosureValue(Node *t, Env *e); 
Value *newEnvValue(Env *parent);
Value *newStringValue(char *s);
Value *newNoneValue();
Value *newBuiltinFun(BuiltinFun f);
void freeValue(Value *v);

char *valueToString(Value *v);

int valueEquals(Value *v1, Value *v2);
Value *valueAdd(Value *v1, Value *v2);
Value *valueSub(Value *v1, Value *v2);
Value *valueMul(Value *v1, Value *v2);
Value *valueDiv(Value *v1, Value *v2);
Value *valueMod(Value *v1, Value *v2);
Value *valueAddEq(Env *e, Node *v1, Node *v2);
int valueCmp(Value *v1, Value *v2);

long getIntFromValue(Value* intValue);
Env* getEnvFromValue(Value* v);
List* getListFromValue(Value* v);

#endif
