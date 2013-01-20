#include "value.h"
#include "ast.h"
#include "env.h"
#include "builtinFun.h"
#include "util.h"

extern Value* globalEnv;
extern List* parseTrees;

Value* builtinLen(List* lst) {
  if(listSize(lst)!=1) error("len only takes exactly one argument\n");
  Value* l = listGet(lst, 0);
  switch(l->type) {
    case LIST_VALUE_TYPE: return newIntValue(listSize(l->data));
    case STRING_VALUE_TYPE: return newIntValue(strlen(l->data));
    default: error("len() cannot apply on value type %d\n", l->type);
  }
}

Value* builtinOrd(List* lst) {
  if(listSize(lst)!=1) error("ord only takes exactly one argument\n");
  Value* v = listGet(lst, 0);
  if(v->type != STRING_VALUE_TYPE || strlen((char*) v->data) != 1)
    error("ord() can only apply to string with length 1\n");
  return newIntValue(*((char*) v->data));
}

Value* builtinSort(List* lst) {
  if(listSize(lst)!=1) error("sort only takes exactly one argument\n");
  Value* v = listGet(lst, 0);
  if(v->type != LIST_VALUE_TYPE) error("sort only applys on list\n");
  listSort(v->data, valueCmp);
  return newNoneValue();
}

Value* builtinStr(List* lst) {
  if(listSize(lst)!=1) error("str only takes exactly one argument\n");
  Value* v = listGet(lst, 0);
  return newStringValue(valueToString(v));
}

Value* builtinChr(List* lst) {
  if(listSize(lst)!=1) error("chr only takes exactly one argument\n");
  Value* v = listGet(lst, 0);
  if(v->type != INT_VALUE_TYPE) error("chr only applys to int\n");
  char* s = (char*) malloc(2);
  s[0] = (long) v->data;
  s[1] = 0;
  return newStringValue(s);
}

Value* builtinPrint(List* lst) {
  int i, n = listSize(lst);
  for(i=0;i<n;i++) {
    if(i) printf(" ");
    printf("%s", valueToString(listGet(lst, i)));
  }
  printf("\n");
  return newNoneValue();
}

Value* builtinRand(List* lst) {
  return newIntValue(rand());
}

Value* builtinParse(List* lst) {
  if(listSize(lst)!=1) error("parse only takes exactly one argument\n");
  Value* v = listGet(lst, 0);
  if(v->type != STRING_VALUE_TYPE) error("parse only applys to string\n");
  char *code = (char*) v->data;
  FILE* f = fmemopen(code, strlen(code), "r");
  yyrestart(f);
  yyparse();
  fclose(f);
  return nodeToListValue(listLast(parseTrees));
}

Value* builtinRead(List* lst){
  if(listSize(lst)!=1) error("read only takes exactly one argument\n");
  Value* v = listGet(lst, 0);
  if(v->type != STRING_VALUE_TYPE) error("read only applys to string\n");
  FILE* f = fopen((char*) v->data, "r");
  if(!f) return newNoneValue();
  fseek(f, 0L, SEEK_END);
  int sz = ftell(f);
  fseek(f, 0L, SEEK_SET);
  char* res = (char*) malloc(sz+1);
  if(fread(res, 1, sz, f)!=sz)
    error("failed to read the whole file of %s\n", v->data);
  res[sz]=0;
  return newStringValue(res);
}

Value* builtinExit(List* lst){
  if(listSize(lst)!=1) error("exit only takes exactly one argument\n");
  Value* v = listGet(lst, 0);
  if(v->type != INT_VALUE_TYPE) error("exit only applys to string\n");
  exit((long) v->data);
}

void registerBuiltinFunctions(Env* e) {
  envPut(e, getIntId("len"), newBuiltinFun(builtinLen));
  envPut(e, getIntId("ord"), newBuiltinFun(builtinOrd));
  envPut(e, getIntId("chr"), newBuiltinFun(builtinChr));
  envPut(e, getIntId("sort"), newBuiltinFun(builtinSort));
  envPut(e, getIntId("str"), newBuiltinFun(builtinStr));
  envPut(e, getIntId("print"), newBuiltinFun(builtinPrint));
  envPut(e, getIntId("rand"), newBuiltinFun(builtinRand));
  envPut(e, getIntId("parse"), newBuiltinFun(builtinParse));
  envPut(e, getIntId("read"), newBuiltinFun(builtinRead));
  envPut(e, getIntId("exit"), newBuiltinFun(builtinExit));
}



