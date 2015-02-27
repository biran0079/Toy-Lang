#include "ast.h"
#include "value.h"
#include "env.h"
#include "execUnit.h"
#include "exception.h"
#include "closure.h"
#include "list.h"
#include "hashTable.h"
#include "eval.h"
#include "tljmp.h"
#include "util.h"
#include "parser.h"
#include "tokenizer.h"
#include "opStack.h"

#include <assert.h>

extern List *rootValues, *parseTrees;
void pushRootValue(Value *v) { listPush(rootValues, v); }

#ifndef USE_LEGACY_EVAL

void eval(Value *ev, Node *p) {
  p->eval(ev, p);
}

void evalStmts(Value *ev, Node *p) {
  List *l = (List *)p->data;
  int i;
  for (i = 0; i < listSize(l); i++) {
    Node *t = listGet(l, i);
    t->eval(ev, t);
  }
}

void evalCall(Value *ev, Node *p) {
  int i, n;
  Node *args = chld(p, 1);
  n = chldNum(args);
  for (i = n-1; i>=0; i--) {
    eval(ev, chld(args, i));
  }
  eval(ev, chld(p, 0));
  while (1) {
    Value *closureValue = opStackPeek(0);
    if (!closureValue) error("fun value is none in function application\n");
    if (closureValue->type == BUILTIN_FUN_VALUE_TYPE) {
      BuiltinFun f = closureValue->data;
      opStackPop(); // poping built in function closure
      f(n);
      return;
    }
    // regular function call
    Closure *c = closureValue->data;
    Node *f = c->f;
    Env *e2 = newEnv(c->e);
    assert(opStackPop()->type == CLOSURE_VALUE_TYPE); // pop closure value
    Node *ids = chld(f, 1);
    if (chldNum(ids) != n) error("%s parameter number incorrect\n", valueToString(closureValue));
    for (i = 0; i < n; i++) {
      envPutLocal(e2, (long)chld(ids, i)->data, opStackPeek(i));
    }
    int ret = tlSetjmp();
    if (0 == ret) {
      Value* ev2 = newEnvValue(e2);
      opStackPopN(n);
      opStackPush(ev2);
      eval(ev2, chld(f, 2));
      // if reach here, no return statement is called, so none is returned
      // return manually
      tlLongjmp(RETURN_MSG_TYPE, newNoneValue());
    } else if (__jmpMsg__.type == RETURN_MSG_TYPE) {
      // return is called. evalReturn is responsible for cleaning up env2
      opStackPush((Value *) __jmpMsg__.data);
      __jmpMsg__.data = 0;
      return;
    } else if (__jmpMsg__.type == TAIL_CALL_MSG_TYPE) {
      // tail recursion call. evalTailRecursion is responsible for cleaning up env2
      List* args = (List*) __jmpMsg__.data;
      n = listSize(args);
      for (i = 0; i < n; i++) {
        opStackPush(listGet(args, i));
      }
      __jmpMsg__.data = 0;
      continue;
    } else {
      // propagate everything out
      tlPropagateJmp();
    }
  }
}

void evalId(Value *ev, Node *p) {
  Env *e = ev->data;
  Value *res = envGet(e, (long)p->data);
  if (!res) res = newNoneValue();
  opStackPush(res);
}

void evalInt(Value *ev, Node *p) {
  opStackPush(newIntValue((long)p->data));
}

void evalList(Value *ev, Node *p) {
  int initSize = listSize(rootValues);
  List *vs = newList();
  Value *res = newListValue(vs);
  opStackPush(res);
  int i, n = chldNum(p);
  for (i = 0; i < n; i++) {
    eval(ev, chld(p, i));
    listPush(vs, opStackPeek(0));
  }
  opStackPopN(n);
}

void evalString(Value *ev, Node *p) {
  opStackPush(newStringValue(copyStr(p->data)));
}

void evalNone(Value *ev, Node *p) {
  opStackPush(newNoneValue());
}

void evalAdd(Value *ev, Node *p) {
  eval(ev, chld(p, 1));
  eval(ev, chld(p, 0));
  Value *res = valueAdd(opStackPeek(0), opStackPeek(1));
  opStackPopN(2);
  opStackPush(res);
}

void evalSub(Value *ev, Node *p) {
  eval(ev, chld(p, 1));
  eval(ev, chld(p, 0));
  Value *res = valueSub(opStackPeek(0), opStackPeek(1));
  opStackPopN(2);
  opStackPush(res);
}

void evalMul(Value *ev, Node *p) {
  eval(ev, chld(p, 1));
  eval(ev, chld(p, 0));
  Value *res = valueMul(opStackPeek(0), opStackPeek(1));
  opStackPopN(2);
  opStackPush(res);
}

void evalDiv(Value *ev, Node *p) {
  eval(ev, chld(p, 1));
  eval(ev, chld(p, 0));
  Value *res = valueDiv(opStackPeek(0), opStackPeek(1));
  opStackPopN(2);
  opStackPush(res);
}

void evalMod(Value *ev, Node *p) {
  eval(ev, chld(p, 1));
  eval(ev, chld(p, 0));
  Value *res = valueMod(opStackPeek(0), opStackPeek(1));
  opStackPopN(2);
  opStackPush(res);
}

void evalGT(Value *ev, Node *p) {
  eval(ev, chld(p, 1));
  eval(ev, chld(p, 0));
  Value* res = newIntValue(valueCmp(opStackPeek(0), opStackPeek(1)) > 0);
  opStackPopN(2);
  opStackPush(res);
}

void evalLT(Value *ev, Node *p) {
  eval(ev, chld(p, 1));
  eval(ev, chld(p, 0));
  Value* res = newIntValue(valueCmp(opStackPeek(0), opStackPeek(1)) < 0);
  opStackPopN(2);
  opStackPush(res);
}

void evalGE(Value *ev, Node *p) {
  eval(ev, chld(p, 1));
  eval(ev, chld(p, 0));
  Value* res = newIntValue(valueCmp(opStackPeek(0), opStackPeek(1)) >= 0);
  opStackPopN(2);
  opStackPush(res);
}

void evalLE(Value *ev, Node *p) {
  eval(ev, chld(p, 1));
  eval(ev, chld(p, 0));
  Value* res = newIntValue(valueCmp(opStackPeek(0), opStackPeek(1)) <= 0);
  opStackPopN(2);
  opStackPush(res);
}

void evalEQ(Value *ev, Node *p) {
  eval(ev, chld(p, 1));
  eval(ev, chld(p, 0));
  Value* res = newIntValue(valueCmp(opStackPeek(0), opStackPeek(1)) == 0);
  opStackPopN(2);
  opStackPush(res);
}
void evalNE(Value *ev, Node *p) {
  eval(ev, chld(p, 1));
  eval(ev, chld(p, 0));
  Value* res = newIntValue(valueCmp(opStackPeek(0), opStackPeek(1)) != 0);
  opStackPopN(2);
  opStackPush(res);
}

void evalAnd(Value *ev, Node *p) {
  eval(ev, chld(p, 1));
  eval(ev, chld(p, 0));
  Value* res = newIntValue(opStackPeek(0)->data && opStackPeek(1)->data);
  opStackPopN(2);
  opStackPush(res);
}

void evalOr(Value *ev, Node *p) {
  eval(ev, chld(p, 1));
  eval(ev, chld(p, 0));
  Value* res = newIntValue(opStackPeek(0)->data || opStackPeek(1)->data);
  opStackPopN(2);
  opStackPush(res);
}

void evalNot(Value *ev, Node *p) {
  eval(ev, chld(p, 0));
  Value* res = newIntValue((long) opStackPeek(0)->data);
  opStackPopN(1);
  opStackPush(res);
}

void evalFun(Value *ev, Node *p) {
  Env *e = ev->data;
  Value *res = newClosureValue(p, ev);
  envPut(e, (long)chld(p, 0)->data, res);
  opStackPush(res);
}

void evalError(Value *ev, Node *p) {
  error("cannot eval unknown node type %d\n", p->type);
}

void evalIf(Value *ev, Node *p) {
  eval(ev, chld(p, 0));
  if (opStackPop()->data) {
    eval(ev, chld(p, 1));
  } else if (chldNum(p) == 3) {
    eval(ev, chld(p, 2));
  }
}

void evalExpList(Value *ev, Node *p) {
  int i, n = chldNum(p);
  for (i = 0; i < n; i++) {
    if (i) opStackPop();
    eval(ev, chld(p, i));
  }
}

void evalListAccess(Value *ev, Node *p) {
  eval(ev, chld(p, 1));
  eval(ev, chld(p, 0));
  Value *v = opStackPeek(0);
  Value *idxValue = opStackPeek(1);
  if (idxValue->type != INT_VALUE_TYPE) error("list index must be int\n");
  long idx = (long)idxValue->data;
  Value *res;
  switch (v->type) {
    case LIST_VALUE_TYPE: {
      List *l = (List *)v->data;
      res = listGet(l, idx);
      break;
    }
    case STRING_VALUE_TYPE: {
      char *s = (char *)v->data;
      char *ss = (char *)tlMalloc(2);
      ss[0] = s[idx];
      ss[1] = 0;
      res = newStringValue(ss);
      break;
    }
    default:
      error("list access does not support value type %s\n", valueToString(v));
  }
  opStackPopN(2);
  opStackPush(res);
}

void evalTailRecursion(Value *ev, Node *p) {
  int i, n;
  List* l = newList();
  Value* lv = newListValue(l);
  opStackPush(lv); // will be cleaned up on long jump
  Node *args = chld(p, 1);
  n = chldNum(args);
  for (i = n-1; i>=0; i--) {
    eval(ev, chld(args, i));
    listPush(l, opStackPop());
  }
  eval(ev, chld(p, 0));
  listPush(l, opStackPop());
  tlLongjmp(TAIL_CALL_MSG_TYPE, lv);
}

void evalReturn(Value *ev, Node *p) {
  error("nor implemented\n");
}
void evalAssign(Value *ev, Node *p) {
  error("nor implemented\n");
}
void evalAddEq(Value *ev, Node *p) {
  error("nor implemented\n");
}
void evalFor(Value *ev, Node* p) {
  error("nor implemented\n");
}
void evalForEach(Value *ev, Node *p) {
  error("nor implemented\n");
}
void evalWhile(Value *ev, Node *p) {
  error("nor implemented\n");
}
void evalContinue(Value *ev, Node *p) {
  error("nor implemented\n");
}
void evalBreak(Value *ev, Node *p) {
  error("nor implemented\n");
}
void evalTime(Value *ev, Node *p) {
  error("nor implemented\n");
}
void evalTry(Value *ev, Node *p) {
  error("nor implemented\n");
}
void evalThrow(Value *ev, Node *p) {
  error("nor implemented\n");
}
void evalAddAdd(Value *ev, Node *p) {
  error("nor implemented\n");
}
void evalLocal(Value *ev, Node *p) {
  error("nor implemented\n");
}
void evalImport(Value *ev, Node *p) {
  error("nor implemented\n");
}
void evalModuleAccess(Value *ev, Node *p) {
  error("nor implemented\n");
}

#else
extern JmpMsg __jmpMsg__;
extern Value *globalEnv;

static void popRootValueTo(int size) {
  if (size > rootValues->size) error("size larger than rootValues->size\n");
  rootValues->size = size;
}

Value* eval(Value* ev, Node *p) {
  Value* res = eval(ev, p);
  opStackPush(res);
  return res;
}

static Value *evalAndPushRoot(Value *ev, Node *p) {
  Value *res = eval(ev, p);
  pushRootValue(res);
  return res;
}

static Value *evalBuiltinFun(Value *e, Node *exp_lst, BuiltinFun f) {
  List *args = newList();
  int i, n = chldNum(exp_lst);
  for (i = 0; i < n; i++) {
    Value *v = evalAndPushRoot(e, chld(exp_lst, i));
    listPush(args, v);
  }
  Value *res = f(args);
  freeList(args);
  return res;
}

Value *evalStmts(Value *ev, Node *p) {
  List *l = (List *)p->data;
  int i;
  for (i = 0; i < listSize(l); i++) {
    Node *t = listGet(l, i);
    eval(ev, t);
  }
  return newNoneValue();
}

Value *evalExpList(Value *ev, Node *p) {
  Value *res = newNoneValue();
  int i, n = chldNum(p);
  for (i = 0; i < n; i++) {
    Node *t = chld(p, i);
    res = eval(ev, t);
  }
  return res;
}

Value *evalNone(Value *ev, Node *p) { return newNoneValue(); }

Value *evalList(Value *ev, Node *p) {
  int initSize = listSize(rootValues);
  List *vs = newList();
  int i, n = chldNum(p);
  for (i = 0; i < n; i++) {
    Value *v = evalAndPushRoot(ev, chld(p, i));
    listPush(vs, v);
  }
  Value *res = newListValue(vs);
  popRootValueTo(initSize);
  return res;
}

Value *evalListAccess(Value *ev, Node *p) {
  int initSize = listSize(rootValues);
  Value *v = evalAndPushRoot(ev, chld(p, 0));
  Value *idxValue = evalAndPushRoot(ev, chld(p, 1));
  if (idxValue->type != INT_VALUE_TYPE) error("list index must be int\n");
  long idx = (long)idxValue->data;
  Value *res;
  switch (v->type) {
    case LIST_VALUE_TYPE: {
      List *l = (List *)v->data;
      res = listGet(l, idx);
      break;
    }
    case STRING_VALUE_TYPE: {
      char *s = (char *)v->data;
      char *ss = (char *)tlMalloc(2);
      ss[0] = s[idx];
      ss[1] = 0;
      res = newStringValue(ss);
      break;
    }
    default:
      error("list access does not support value type %s\n", valueToString(v));
  }
  popRootValueTo(initSize);
  return res;
}

Value *evalTailRecursion(Value *ev, Node *p) {
  int i, initSize = listSize(rootValues);
  Env *e = ev->data;
  Value *closureValue = evalAndPushRoot(ev, chld(p, 0));
  if (closureValue->type == BUILTIN_FUN_VALUE_TYPE) {
    Value *res = evalBuiltinFun(ev, chld(p, 1), closureValue->data);
    popRootValueTo(initSize);
    return res;
  }
  if (closureValue->type != CLOSURE_VALUE_TYPE)
    error("only closure value can be called\n");
  Closure *c = closureValue->data;
  Node *f = c->f, *args = chld(p, 1);
  Env *e2 = newEnv(c->e);
  Value *ev2 = newEnvValue(e2);
  pushRootValue(ev2);
  Node *ids = chld(f, 1);
  if (chldNum(ids) != chldNum(args))
    error("%s parameter number incorrect\n", valueToString(closureValue));
  for (i = 0; i < chldNum(ids); i++) {
    Node *t = chld(args, i);
    envPutLocal(e2, (long)chld(ids, i)->data, eval(ev, t));
  }
  Value *res = newClosureValue(f, ev2);
  tlLongjmp(e->retState, TAIL_CALL_MSG_TYPE, res);
  // actuall call is handled by CALL_TYPE
  return 0;  // never reach here;
}

Value *evalCall(Value *ev, Node *p) {
  int i, initSize = listSize(rootValues);
  Env *e = ev->data;
  Value *closureValue = evalAndPushRoot(ev, chld(p, 0));
  if (!closureValue) error("fun value is none in function application\n");
  if (closureValue->type == BUILTIN_FUN_VALUE_TYPE) {
    Value *res = evalBuiltinFun(ev, chld(p, 1), closureValue->data);
    popRootValueTo(initSize);
    return res;
  }
  Closure *c = closureValue->data;
  Node *f = c->f, *args = chld(p, 1);
  Env *e2 = newEnv(c->e);
  Value *ev2 = newEnvValue(e2);
  pushRootValue(ev2);
  Node *ids = chld(f, 1);
  if (chldNum(ids) != chldNum(args))
    error("%s parameter number incorrect\n", valueToString(closureValue));
  for (i = 0; i < chldNum(ids); i++) {
    Node *t = chld(args, i);
    envPutLocal(e2, (long)chld(ids, i)->data, eval(ev, t));
  }
  while (1) {
    int loopN = envNumOfLoopStates(e), exN = envNumOfExceptionStates(e);
    int ret = setjmp(e2->retState);
    if (!ret) {
      Node *t = chld(f, 2);
      eval(ev2, t);
      // if reach here, no return statement is called, so none is returned
      popRootValueTo(initSize);
      return newNoneValue();
    } else {
      envRestoreStates(e, loopN, exN);
      if (__jmpMsg__.type == RETURN_MSG_TYPE) {
        // return is called;
        Value *returnValue = __jmpMsg__.data;
        popRootValueTo(initSize);
        return returnValue;
      }
      if (__jmpMsg__.type == TAIL_CALL_MSG_TYPE) {
        // tail recursive call, reuse environment e2
        Value *closureValue = __jmpMsg__.data;
        Closure *tc = closureValue->data;
        f = tc->f;
        ev2 = tc->e;
        e2 = ev2->data;
        popRootValueTo(initSize);
        pushRootValue(ev2);  // ev2 is all we need to keep
        continue;
      }
      if (__jmpMsg__.type == EXCEPTION_MSG_TYPE) {
        // exception passed out
        Value *v = __jmpMsg__.data;
        popRootValueTo(initSize);
        throwValue(e, v);
      } else {
        error("unknown  return value from longjmp in function call\n");
      }
    }
  }
}

Value *evalReturn(Value *ev, Node *p) {
  Env *e = ev->data;
  if (e->parent == newNoneValue())
    error("cannot call return outside a function.\n");
  Node *t = chld(p, 0);
  tlLongjmp(e->retState, RETURN_MSG_TYPE, eval(ev, t));
  return 0;  // never reach here;
}

Value *evalId(Value *ev, Node *p) {
  Env *e = ev->data;
  Value *res = envGet(e, (long)p->data);
  return res ? res : newNoneValue();
}

Value *evalInt(Value *ev, Node *p) { return newIntValue((long)p->data); }

Value *evalString(Value *ev, Node *p) {
  return newStringValue(copyStr(p->data));
}

Value *evalAssign(Value *ev, Node *p) {
  Node *left = chld(p, 0);
  int i, initSize = listSize(rootValues);
  Env *e = ev->data;
  switch (left->type) {
    case LIST_ACCESS_TYPE: {
      Value *lv = evalAndPushRoot(ev, chld(left, 0));
      if (lv->type != LIST_VALUE_TYPE) error("= only applys to list\n");
      List *l = lv->data;
      Value *idx = evalAndPushRoot(ev, chld(left, 1));
      if (idx->type != INT_VALUE_TYPE) error("list index must be int\n");
      Node *t = chld(p, 1);
      listSet(l, (long)idx->data, eval(ev, t));
      Value *res = listGet(l, (long)idx->data);
      popRootValueTo(initSize);
      return res;
    }
    case ID_TYPE: {
      Node *t = chld(p, 1);
      Value *res = eval(ev, t);
      envPut(e, (long)left->data, res);
      return res;
    }
    case MODULE_ACCESS_TYPE: {
      Node *t = chld(p, 1);
      Value *res = eval(ev, t);
      int n = chldNum(left);
      Value *env = ev;
      for (i = 0; i < n - 1; i++) {
        long id = (long)chld(left, i)->data;
        if (env->type != ENV_VALUE_TYPE)
          error("module access must use env type, %s get\n",
                valueToString(env));
        env = envGet(env->data, id);
      }
      envPutLocal(env->data, (long)chld(left, i)->data, res);
      return res;
    }
    default:
      error("left hand side of = must be a left value\n");
  }
  return 0;  // never reach here;
}

Value *evalAddEq(Value *ev, Node *p) {
  int i, initSize = listSize(rootValues);
  Env *e = ev->data;
  Node *left = chld(p, 0);
  switch (left->type) {
    case LIST_ACCESS_TYPE: {
      Value *lv = evalAndPushRoot(ev, chld(left, 0));
      if (lv->type != LIST_VALUE_TYPE) error("+= only applys to list\n");
      List *l = lv->data;
      Value *idx = evalAndPushRoot(ev, chld(left, 1));
      if (idx->type != INT_VALUE_TYPE) error("list index must be int\n");
      Value *e1 = listGet(l, (long)idx->data);
      Value *e2 = evalAndPushRoot(ev, chld(p, 1));
      Value *res = newNoneValue();
      switch (e1->type) {
        case INT_VALUE_TYPE:
        case STRING_VALUE_TYPE:
          res = valueAdd(e1, e2);
          listSet(l, (long)idx->data, res);
          break;
        case LIST_VALUE_TYPE:
          if (e2->type == LIST_VALUE_TYPE) {
            List *l = (List *)e2->data;
            for (i = 0; i < listSize(l); i++)
              listPush((List *)e1->data, listGet(l, i));
          } else {
            listPush((List *)e1->data, e2);
          }
          res = e1;
          break;
        default:
          error("unknown type for operator +=: %d\n", e1->type);
      }
      popRootValueTo(initSize);
      return res;
    }
    case ID_TYPE: {
      long key = (long)chld(p, 0)->data;
      Value *e1 = envGet(e, key);
      Value *e2 = evalAndPushRoot(ev, chld(p, 1));
      Value *res = newNoneValue();
      switch (e1->type) {
        case INT_VALUE_TYPE:
        case STRING_VALUE_TYPE:
          res = valueAdd(e1, e2);
          envPut(e, key, res);
          break;
        case LIST_VALUE_TYPE:
          if (e2->type == LIST_VALUE_TYPE) {
            List *l = (List *)e2->data;
            for (i = 0; i < listSize(l); i++)
              listPush((List *)e1->data, listGet(l, i));
          } else {
            listPush((List *)e1->data, e2);
          }
          res = e1;
          break;
        default:
          error("unknown type for operator +=\n");
      }
      popRootValueTo(initSize);
      return res;
    }
    default:
      error("left hand side of += must be left value\n");
  }
  return 0;  // never reach here
}

Value *evalAdd(Value *ev, Node *p) {
  int initSize = listSize(rootValues);
  Value *e1 = evalAndPushRoot(ev, chld(p, 0));
  Value *e2 = evalAndPushRoot(ev, chld(p, 1));
  Value *res = valueAdd(e1, e2);
  popRootValueTo(initSize);
  return res;
}

Value *evalSub(Value *ev, Node *p) {
  int initSize = listSize(rootValues);
  Value *e1 = evalAndPushRoot(ev, chld(p, 0));
  Value *e2 = evalAndPushRoot(ev, chld(p, 1));
  Value *res = valueSub(e1, e2);
  popRootValueTo(initSize);
  return res;
}

Value *evalMul(Value *ev, Node *p) {
  int initSize = listSize(rootValues);
  Value *e1 = evalAndPushRoot(ev, chld(p, 0));
  Value *e2 = evalAndPushRoot(ev, chld(p, 1));
  Value *res = valueMul(e1, e2);
  popRootValueTo(initSize);
  return res;
}

Value *evalDiv(Value *ev, Node *p) {
  int initSize = listSize(rootValues);
  Value *e1 = evalAndPushRoot(ev, chld(p, 0));
  Value *e2 = evalAndPushRoot(ev, chld(p, 1));
  Value *res = valueDiv(e1, e2);
  popRootValueTo(initSize);
  return res;
}

Value *evalMod(Value *ev, Node *p) {
  int initSize = listSize(rootValues);
  Value *e1 = evalAndPushRoot(ev, chld(p, 0));
  Value *e2 = evalAndPushRoot(ev, chld(p, 1));
  Value *res = valueMod(e1, e2);
  popRootValueTo(initSize);
  return res;
}

Value *evalIf(Value *ev, Node *p) {
  Node *t = chld(p, 0);
  if (eval(ev, t)->data) {
    t = chld(p, 1);
    return eval(ev, t);
  } else if (chldNum(p) == 3) {
    t = chld(p, 2);
    return eval(ev, t);
  }
  return newNoneValue();
}

Value *evalFor(Value *ev, Node *p) {
  Env *e = ev->data;
  jmp_buf buf;
  listPush(envGetLoopStates(e), buf);
  Node *t1 = chld(p, 0), *t2 = chld(p, 1), *t3 = chld(p, 2);
  for (eval(ev, t1); eval(ev, t2)->data; eval(ev, t3)) {
    int loopN = envNumOfLoopStates(e), exN = envNumOfExceptionStates(e);
    if (setjmp(buf) == 0) {
      Node *t4 = chld(p, 3);
      eval(ev, t4);
    } else {
      envRestoreStates(e, loopN, exN);
      JmpMsg *msg = &__jmpMsg__;
      if (msg->type == CONTINUE_MSG_TYPE)
        continue;
      else if (msg->type == BREAK_MSG_TYPE)
        break;
      else
        error("unexpected jmpmsg type: %d\n", msg->type);
    }
  }
  listPop(envGetLoopStates(e));
  return newNoneValue();
}

Value *evalForEach(Value *ev, Node *p) {
  int i, initSize = listSize(rootValues);
  Env *e = ev->data;
  jmp_buf buf;
  listPush(envGetLoopStates(e), buf);
  Node *id = chld(p, 0);
  Value *lv = evalAndPushRoot(ev, chld(p, 1));
  if (id->type != ID_TYPE || lv->type != LIST_VALUE_TYPE) {
    error("param type incorrect for for( : ) statement\n ");
  }
  List *l = (List *)lv->data;
  int len = listSize(l);
  for (i = 0; i < len; i++) {
    envPut(e, (long)id->data, listGet(l, i));
    int loopN = envNumOfLoopStates(e), exN = envNumOfExceptionStates(e);
    if (setjmp(buf) == 0) {
      Node *t = chld(p, 2);
      eval(ev, t);
    } else {
      envRestoreStates(e, loopN, exN);
      JmpMsg *msg = &__jmpMsg__;
      if (msg->type == CONTINUE_MSG_TYPE)
        continue;
      else if (msg->type == BREAK_MSG_TYPE)
        break;
      else
        error("unexpected jmpmsg type: %d\n", msg->type);
    }
  }
  listPop(envGetLoopStates(e));
  popRootValueTo(initSize);
  return newNoneValue();
}

Value *evalWhile(Value *ev, Node *p) {
  int initSize = listSize(rootValues);
  Env *e = ev->data;
  jmp_buf buf;
  listPush(envGetLoopStates(e), buf);
  Node *t = chld(p, 0);
  while (eval(ev, t)->data) {
    int loopN = envNumOfLoopStates(e), exN = envNumOfExceptionStates(e);
    if (setjmp(buf) == 0) {
      Node *t = chld(p, 1);
      eval(ev, t);
    } else {
      envRestoreStates(e, loopN, exN);
      JmpMsg *msg = &__jmpMsg__;
      if (msg->type == CONTINUE_MSG_TYPE)
        continue;
      else if (msg->type == BREAK_MSG_TYPE) {
        break;
      } else
        error("unexpected jmpmsg type: %d\n", msg->type);
    }
  }
  listPop(envGetLoopStates(e));
  return newNoneValue();
}

Value *evalContinue(Value *ev, Node *p) {
  Env *e = ev->data;
  tlLongjmp(listLast(envGetLoopStates(e)), CONTINUE_MSG_TYPE, 0);
  return 0;  // never reach here
}

Value *evalBreak(Value *ev, Node *p) {
  Env *e = ev->data;
  tlLongjmp(listLast(envGetLoopStates(e)), BREAK_MSG_TYPE, 0);
  return 0;  // never reach here
}

Value *evalGT(Value *ev, Node *p) {
  return newIntValue(valueCmp(evalAndPushRoot(ev, chld(p, 0)),
                              evalAndPushRoot(ev, chld(p, 1))) > 0);
}

Value *evalLT(Value *ev, Node *p) {
  return newIntValue(valueCmp(evalAndPushRoot(ev, chld(p, 0)),
                              evalAndPushRoot(ev, chld(p, 1))) < 0);
}

Value *evalGE(Value *ev, Node *p) {
  return newIntValue(valueCmp(evalAndPushRoot(ev, chld(p, 0)),
                              evalAndPushRoot(ev, chld(p, 1))) >= 0);
}

Value *evalLE(Value *ev, Node *p) {
  return newIntValue(valueCmp(evalAndPushRoot(ev, chld(p, 0)),
                              evalAndPushRoot(ev, chld(p, 1))) <= 0);
}

Value *evalEQ(Value *ev, Node *p) {
  return newIntValue(valueCmp(evalAndPushRoot(ev, chld(p, 0)),
                              evalAndPushRoot(ev, chld(p, 1))) == 0);
}

Value *evalNE(Value *ev, Node *p) {
  return newIntValue(!valueEquals(evalAndPushRoot(ev, chld(p, 0)),
                                  evalAndPushRoot(ev, chld(p, 1))));
}

Value *evalAnd(Value *ev, Node *p) {
  Node *t1 = chld(p, 0), *t2 = chld(p, 1);
  return newIntValue(eval(ev, t1)->data && eval(ev, t2)->data);
}

Value *evalOr(Value *ev, Node *p) {
  Node *t1 = chld(p, 0), *t2 = chld(p, 1);
  return newIntValue(eval(ev, t1)->data || eval(ev, t2)->data);
}

Value *evalNot(Value *ev, Node *p) {
  Node *t = chld(p, 0);
  return newIntValue(!eval(ev, t)->data);
}

Value *evalFun(Value *ev, Node *p) {
  Env *e = ev->data;
  Value *res = newClosureValue(p, ev);
  envPut(e, (long)chld(p, 0)->data, res);
  return res;
}

Value *evalTime(Value *ev, Node *p) {
  clock_t st = clock();
  Node *t = chld(p, 0);
  Value *res = eval(ev, t);
  fprintf(stderr, "time: %lf secs\n", (clock() - st) * 1.0 / CLOCKS_PER_SEC);
  return newNoneValue();
}

Value *evalTry(Value *ev, Node *p) {
  Env *e = ev->data;
  ExecUnit *finally = chldNum(p) == 4 ? newExecUnit(ev, chld(p, 3)) : 0;
  Exception *ex = newException(finally);
  envPushExceptionStates(e, ex);
  Node *tryBlock = chld(p, 0);
  long catchId = (long)chld(p, 1)->data;
  Node *catchBlock = chld(p, 2);
  int loopN = envNumOfLoopStates(e), exN = envNumOfExceptionStates(e);
  if (setjmp(ex->buf) == 0) {
    // try block
    eval(ev, tryBlock);
  } else {
    // catch block
    envRestoreStates(e, loopN, exN);
    if (setjmp(ex->buf) == 0) {
      Value *v = __jmpMsg__.data;
      envPutLocal(e, catchId, v);
      eval(ev, catchBlock);
    } else {
      // exception from catch or finally block
      Value *v = __jmpMsg__.data;
      pushRootValue(v);
      envRestoreStates(e, loopN, exN);
      envPopExceptionStates(e);
      throwValue(e, v);
    }
  }
  envPopExceptionStates(e);
  return newNoneValue();
}

Value *evalThrow(Value *ev, Node *p) {
  Env *e = ev->data;
  Node *t = chld(p, 0);
  Value *v = eval(ev, t);
  throwValue(e, v);
  return 0;  // never reach here
}

Value *evalAddAdd(Value *ev, Node *p) {
  // i++ type
  Env *e = ev->data;
  int initSize = listSize(rootValues);
  Node *left = chld(p, 0);
  switch (left->type) {
    case ID_TYPE: {
      long id = (long)left->data;
      Value *i = envGet(e, id);
      if (i->type != INT_VALUE_TYPE)
        error("++ can only apply to int\n%s\n",
              valueToString(nodeToListValue(p)));
      envPut(e, id, newIntValue((long)i->data + 1));
      return i;
    }
    case LIST_ACCESS_TYPE: {
      Value *lv = evalAndPushRoot(ev, chld(left, 0));
      if (lv->type != LIST_VALUE_TYPE)
        error("++ only applys to left value\n%s\n",
              valueToString(nodeToListValue(p)));
      List *l = (List *)lv->data;
      Value *idx = evalAndPushRoot(ev, chld(left, 1));
      if (idx->type != INT_VALUE_TYPE) error("index of list must be int\n");
      long idxv = (long)idx->data;
      Value *i = listGet(l, idxv);
      if (i->type != INT_VALUE_TYPE) error("++ only applys on int\n");
      listSet(l, idxv, newIntValue((long)i->data + 1));
      popRootValueTo(initSize);
      return i;
    }
    default:
      error("++ does not support value type %s\n",
            nodeTypeToString(left->type));
  }
  return 0;  // never reach here
}

Value *evalLocal(Value *ev, Node *p) {
  Node *ids = chld(p, 0);
  int i, n = chldNum(ids);
  Env *e = ev->data;
  for (i = 0; i < n; i++) {
    envPutLocal(e, (long)chld(ids, i)->data, newNoneValue());
  }
  return newNoneValue();
}

Value *evalImport(Value *ev, Node *p) {
  int i, initSize = listSize(rootValues);
  Env *e = ev->data;
  Node *id = chld(p, 0);
  char *s = catStr(getStrId((long)id->data), ".tl");
  FILE *f = openFromPath(s, "r");
  if (!f) error("cannot open file %s\n", s);
#ifdef USE_YY_PARSER
  yyrestart(f);
  if (yyparse()) error("failed to parse %s\n", s);
  tlFree(s);
#else
  if (!parse(tokenize(readFile(f)))) error("failed to parse %s\n", s);
#endif
  Value *res = newEnvValue(newEnv(globalEnv));
  pushRootValue(res);
  Node *t = listLast(parseTrees);
  eval(res, t);
  envPut(e, (long)id->data, res);
  popRootValueTo(initSize);
  return newNoneValue();
}

Value *evalModuleAccess(Value *ev, Node *p) {
  int i, n = chldNum(p);
  Value *res = ev;
  for (i = 0; i < n; i++) {
    long id = (long)chld(p, i)->data;
    if (res->type != ENV_VALUE_TYPE)
      error("module access must use env type, %s get\n", valueToString(res));
    res = envGet(res->data, id);
  }
  return res;
}

Value *evalError(Value *ev, Node *p) {
  error("cannot eval unknown node type %d\n", p->type);
  return 0;  // never reach here
}

void throwValue(Env *e, Value *v) {
  if (envLastExceptionState(e)) {
    // currently in try block
    tlLongjmp(envLastExceptionState(e)->buf, EXCEPTION_MSG_TYPE, v);
  } else {
    // If has parent stack frame, pass to it. Report error otherwise
    if (e->parent != newNoneValue()) {
      tlLongjmp(e->retState, EXCEPTION_MSG_TYPE, v);
    } else {
      error("uncaught exception: %s\n", valueToString(v));
    }
  }
}
#endif
