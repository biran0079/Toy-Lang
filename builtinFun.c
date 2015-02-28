#include "parser.h"
#include "tokenizer.h"
#include "value.h"
#include "ast.h"
#include "env.h"
#include "eval.h"
#include "builtinFun.h"
#include "util.h"
#include "opStack.h"

extern Value *globalEnv;
extern List *parseTrees;


#define CHECK_ARG_NUM(num, name) if (n != num) error(#num" argument required for "#name"\n");
void builtinLen(int n) {
  CHECK_ARG_NUM(1, len());
  Value *l = opStackPeek(0);
  Value *res;
  switch (l->type) {
    case LIST_VALUE_TYPE:
      res = newIntValue(listSize(l->data));
      break;
    case STRING_VALUE_TYPE:
      res = newIntValue(strlen(l->data));
      break;
    default:
      error("len() cannot apply on value type %d\n", l->type);
  }
  opStackPopN(n);
  opStackPush(res);
}

void builtinOrd(int n) {
  CHECK_ARG_NUM(1, ord());
  Value *v = opStackPeek(0);
  if (v->type != STRING_VALUE_TYPE || strlen((char *)v->data) != 1)
    error("ord() can only apply to string with length 1, got %s\n",
          valueToString(v));
  Value* res = newIntValue(*((char *)v->data));
  opStackPopN(n);
  opStackPush(res);
}

void builtinSort(int n) {
  CHECK_ARG_NUM(1, sort());
  Value *v = opStackPeek(0);
  if (v->type != LIST_VALUE_TYPE) error("sort only applys on list\n");
  listSort(v->data, valueCmp);
  opStackPopN(n);
  opStackPush(newNoneValue());
}

void builtinStr(int n) {
  CHECK_ARG_NUM(1, str());
  Value *v = opStackPeek(0);
  Value *res = newStringValue(valueToString(v));
  opStackPopN(n);
  opStackPush(res);
}

void builtinChr(int n) {
  CHECK_ARG_NUM(1, chr());
  Value *v = opStackPeek(0);
  if (v->type != INT_VALUE_TYPE) error("chr only applys to int\n");
  char *s = (char *)tlMalloc(2);
  s[0] = (long)v->data;
  s[1] = 0;
  Value* res = newStringValue(s);
  opStackPopN(n);
  opStackPush(res);
}

void builtinPrint(int n) {
  int i;
  for (i = 0; i < n; i++) {
    if (i) printf(" ");
    char *s = valueToString(opStackPeek(i));
    printf("%s", s);
    tlFree(s);
  }
  printf("\n");
  Value* res = newNoneValue();
  opStackPopN(n);
  opStackPush(res);
}

void builtinRand(int n) { 
  CHECK_ARG_NUM(0, rand());
  opStackPopN(n);
  opStackPush(newIntValue(rand()));
}

void builtinParse(int n) {
  CHECK_ARG_NUM(1, parse());
  Value *v = opStackPeek(0);
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
  Value* res = nodeToListValue(listLast(parseTrees));
  opStackPopN(n);
  opStackPush(res);
}

void builtinRead(int n) {
  CHECK_ARG_NUM(1, read());
  Value *v = opStackPeek(0);
  if (v->type != STRING_VALUE_TYPE) {
    error("read only applys to string, got type %d\n", v->type);
  }
  char *s = readFileWithPath((char *)v->data);
  Value* res = newNoneValue();;
  if (s) res = newStringValue(s);
  opStackPopN(n);
  opStackPush(res);
}

void builtinExit(int n) {
  CHECK_ARG_NUM(1, read());
  Value *v = opStackPeek(0);
  if (v->type != INT_VALUE_TYPE) error("exit only applys to string\n");
  exit((long)v->data);
}

int sysArgc;
char **sysArgv;

void builtinSysargs(int n) {
  CHECK_ARG_NUM(0, sysargs());
  List *lst = newList();
  Value *res = newListValue(lst);
  opStackPopN(n);
  opStackPush(res);
  int i;
  for (i = 0; i < sysArgc; i++) {
    listPush(lst, newStringValue(copyStr(sysArgv[i])));
  }
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
