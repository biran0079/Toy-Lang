#include "parser.h"
#include "tokenizer.h"
#include "value.h"
#include "ast.h"
#include "env.h"
#include "eval.h"
#include "builtinFun.h"
#include "util.h"

extern Value *globalEnv;
extern List *parseTrees;

Value *builtinLen(List *lst) {
  if (listSize(lst) != 1) error("len only takes exactly one argument\n");
  Value *l = listGet(lst, 0);
  switch (l->type) {
    case LIST_VALUE_TYPE:
      return newIntValue(listSize(l->data));
    case STRING_VALUE_TYPE:
      return newIntValue(strlen(l->data));
    default:
      error("len() cannot apply on value type %d\n", l->type);
  }
  return 0;  // never reach here
}

Value *builtinOrd(List *lst) {
  if (listSize(lst) != 1) error("ord only takes exactly one argument\n");
  Value *v = listGet(lst, 0);
  if (v->type != STRING_VALUE_TYPE || strlen((char *)v->data) != 1)
    error("ord() can only apply to string with length 1, got %s\n",
          valueToString(v));
  return newIntValue(*((char *)v->data));
}

Value *builtinSort(List *lst) {
  if (listSize(lst) != 1) error("sort only takes exactly one argument\n");
  Value *v = listGet(lst, 0);
  if (v->type != LIST_VALUE_TYPE) error("sort only applys on list\n");
  listSort(v->data, valueCmp);
  return newNoneValue();
}

Value *builtinStr(List *lst) {
  if (listSize(lst) != 1) error("str only takes exactly one argument\n");
  Value *v = listGet(lst, 0);
  return newStringValue(valueToString(v));
}

Value *builtinChr(List *lst) {
  if (listSize(lst) != 1) error("chr only takes exactly one argument\n");
  Value *v = listGet(lst, 0);
  if (v->type != INT_VALUE_TYPE) error("chr only applys to int\n");
  char *s = (char *)tlMalloc(2);
  s[0] = (long)v->data;
  s[1] = 0;
  return newStringValue(s);
}

Value *builtinPrint(List *lst) {
  int i, n = listSize(lst);
  for (i = 0; i < n; i++) {
    if (i) printf(" ");
    char *s = valueToString(listGet(lst, i));
    printf("%s", s);
    tlFree(s);
  }
  printf("\n");
  return newNoneValue();
}

Value *builtinRand(List *lst) { return newIntValue(rand()); }

Value *builtinParse(List *lst) {
  if (listSize(lst) != 1) error("parse only takes exactly one argument\n");
  Value *v = listGet(lst, 0);
  if (v->type != STRING_VALUE_TYPE) error("parse only applys to string\n");
  char *code = (char *)v->data;
#if USE_YY_PARSER
#ifdef _WIN32
  error("Win32 does not support built in parse with YY parser.");
#else
  FILE *f = fmemopen(code, strlen(code), "r");
  yyrestart(f);
  if (yyparse()) error("failed to parse code %s\n", code);
  fclose(f);
#endif
#else
  if (!parse(tokenize(code))) error("failed to parse %s\n", code);
#endif
  return nodeToListValue(listLast(parseTrees));
}

Value *builtinRead(List *lst) {
  if (listSize(lst) != 1) error("read only takes exactly one argument\n");
  Value *v = listGet(lst, 0);
  if (v->type != STRING_VALUE_TYPE) {
    error("read only applys to string, got type %d\n", v->type);
  }
  char *res = readFileWithPath((char *)v->data);
  if (res) return newStringValue(res);
  return newNoneValue();
}

Value *builtinExit(List *lst) {
  if (listSize(lst) != 1) error("exit only takes exactly one argument\n");
  Value *v = listGet(lst, 0);
  if (v->type != INT_VALUE_TYPE) error("exit only applys to string\n");
  exit((long)v->data);
}

int sysArgc;
char **sysArgv;

Value *builtinSysargs(List *l) {
  List *lst = newList();
  Value *res = newListValue(lst);
  pushRootValue(res);
  int i;
  for (i = 0; i < sysArgc; i++) {
    Value *s = newStringValue(copyStr(sysArgv[i]));
    listPush(lst, s);
  }
  return res;
}

void registerBuiltinFunctions(Env *e) {
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
  envPut(e, getIntId("sysArgs"), newBuiltinFun(builtinSysargs));
}
