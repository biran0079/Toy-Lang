#include "value.h"
#include "env.h"
#include "builtinFun.h"
#include "util.h"

extern Value* globalEnv;

Value* builtInLen(List* lst) {
  if(listSize(lst)!=1) error("len only takes exactly one argument\n");
  Value* l = listGet(lst, 0);
  switch(l->type) {
    case LIST_VALUE_TYPE: return newIntValue(listSize(l->data));
    case STRING_VALUE_TYPE: return newIntValue(strlen(l->data));
    default: error("len() cannot apply on value type %d\n", l->type);
  }
}

Value* builtInOrd(List* lst) {
  if(listSize(lst)!=1) error("ord only takes exactly one argument\n");
  Value* v = listGet(lst, 0);
  if(v->type != STRING_VALUE_TYPE || strlen((char*) v->data) != 1)
    error("ord() can only apply to string with length 1\n");
  return newIntValue(*((char*) v->data));
}

Value* builtInSort(List* lst) {
  if(listSize(lst)!=1) error("sort only takes exactly one argument\n");
  Value* v = listGet(lst, 0);
  if(v->type != LIST_VALUE_TYPE) error("sort only applys on list\n");
  listSort(v->data, valueCmp);
  return newNoneValue();
}

Value* builtInStr(List* lst) {
  if(listSize(lst)!=1) error("str only takes exactly one argument\n");
  Value* v = listGet(lst, 0);
  return newStringValue(valueToString(v));
}

void registerBuiltinFunctions() {
  Env* e = globalEnv->data;
  envPut(e, "len", newBuiltinFun(builtInLen));
  envPut(e, "ord", newBuiltinFun(builtInOrd));
  envPut(e, "sort", newBuiltinFun(builtInSort));
  envPut(e, "str", newBuiltinFun(builtInStr));
}



