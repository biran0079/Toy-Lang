#include "ast.h"
#include "value.h"
#include "env.h"
#include "closure.h"
#include "list.h"
#include "hashTable.h"
#include "eval.h"
#include "util.h"
#include "parser.h"
#include "tokenizer.h"
#include "opStack.h"

#include <assert.h>

extern List *parseTrees;
extern Value *globalEnv;

static List* pointersInEvalStack;

void inStackPointerUpdateAddr(HashTable* addrMap) {
  int i, n = listSize(pointersInEvalStack);
  for (i = 0; i < n; i++) {
    Value** p = listGet(pointersInEvalStack, i);
    Value* newAddr = hashTableGet(addrMap, *p);
#ifdef DEBUG_GC
    printf("updating in stack ptr %p -> %p\n", *p, newAddr);
#endif
    assert(newAddr);
    *p = newAddr;
  }
}

static void pushPointerInEvalStack(Value** p) {
  listPush(pointersInEvalStack, p);
}

static void popPointerInEvalStack(Value** p) {
  assert(p == listPop(pointersInEvalStack));
}

void initEval() {
  pointersInEvalStack = newList();
  pushPointerInEvalStack(&globalEnv);
}

void cleanupEval() {
  popPointerInEvalStack(&globalEnv);
  assert(listSize(pointersInEvalStack) == 0);
  freeList(pointersInEvalStack);
}

EvalResult *newEvalResult(EvalResultType t, Value *v) {
  newEvalResultC++;
  EvalResult *res = MALLOC(EvalResult);
  res->type = t;
  res->value = v;
  return res;
}

void freeEvalResult(EvalResult *er) {
  freeEvalResultC++;
  tlFree(er);
}

/**
 * Assumptions:
 * 1. one value will be pushed to op stack if a eval returns 0
 * 2. no value will be pushed to op stack if eval returns non-zero
 * 3. all eval call's return value must be checked
 * 4. anything in op stack will not be GCed
 * 5. each opStackSave must be paired by exactly on opStackRestore before returning
 * 6. any usage of Value* type in eval call shuold be pushed to pointersInEvalStack to survive GC
 * 7. no GC in env operations and op-stack operations
 */
EvalResult *eval(Value *ev, Node *p) {
  int beforeStackSize = opStackSize();
  EvalResult *er = p->eval(ev, p);
  if (er) {
    if (opStackSize() != beforeStackSize) printAst(p);
    assert(opStackSize() == beforeStackSize);
  } else {
    if (opStackSize() != beforeStackSize + 1) printAst(p);
    assert(opStackSize() == beforeStackSize + 1);
  }
  return er;
}

#define EVAL_HEADER pushPointerInEvalStack(&ev)
#define EVAL_RETURN(er) do {popPointerInEvalStack(&ev); return er;} while(0)

EvalResult *evalStmts(Value *ev, Node *p) {
  EVAL_HEADER;
  List *l = (List *)p->data;
  int i;
  for (i = 0; i < listSize(l); i++) {
    EvalResult *er = eval(ev, listGet(l, i));
    if (er) {
      EVAL_RETURN(er);
    } else {
      opStackPop();
    }
  }
  opStackPush(newNoneValue());
  EVAL_RETURN(0);
}

EvalResult *evalCall(Value *ev, Node *p) {
  EVAL_HEADER;
  int i, n;
  Node *args = chld(p, 1);
  n = chldNum(args);
  int beforeStackSize = opStackSize();
  for (i = n - 1; i >= 0; i--) {
    EvalResult *er = eval(ev, chld(args, i));
    if (er) {
      assert(er->type == EXCEPTION_RESULT);
      opStackPopTo(beforeStackSize);
      EVAL_RETURN(er);
    }
  }
  EvalResult *er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPopTo(beforeStackSize);
    EVAL_RETURN(er);
  }
  while (1) {
    if (opStackPeek(0)->type == BUILTIN_FUN_VALUE_TYPE) {
      BuiltinFun f = opStackPeek(0)->data;
      opStackPop();  // pop closure
      f(n);
      // always success and result is in stack
      EVAL_RETURN(0);
    }
    // regular function call
    Closure *c = opStackPeek(0)->data;
    Node *f = c->f;

    // This is tricky, er->parent is a pointer that GC has no idea how to update
    Value* ev2 = newEnvValue(&(c->e));
    Env *e2 = getEnvFromValue(ev2);

    Node *ids = chld(f, 1);
    if (chldNum(ids) != n)
      error("%s parameter number incorrect\n", valueToString(opStackPeek(0)));

    for (i = 0; i < n; i++)
      envPutLocal(e2, (long)chld(ids, i)->data, opStackPeek(i + 1));

    opStackPopNPush(n + 1, ev2);


    EvalResult *er = eval(opStackPeek(0), chld(f, 2));

    if (er) {
      switch (er->type) {
        case EXCEPTION_RESULT: {
          opStackPopTo(beforeStackSize);
          EVAL_RETURN(er);
        }
        case TAIL_RECURSION_RESULT: {
          opStackPopTo(beforeStackSize);
          List *l = (List *)er->value->data;
          n = listSize(l);
          for (i = 0; i < n; i++) {
            opStackPush(listGet(l, i));
          }
          n--;  // one of the args is closure
          freeEvalResult(er);
          continue;
        }
        case RETURN_RESULT: {
          opStackPopToPush(beforeStackSize, er->value);
          freeEvalResult(er);
          EVAL_RETURN(0);
        }
        case CONTINUE_RESULT:
        case BREAK_RESULT:
          error("continue or break outside loop\n");
      }
    } else {
      // no return called, always return none
      opStackPopToPush(beforeStackSize, newNoneValue());
      EVAL_RETURN(0);
    }
  }
}

EvalResult *evalId(Value *ev, Node *p) {
  EVAL_HEADER;
  Env *e = ev->data;
  opStackPush(envGet(e, (long)p->data));
  EVAL_RETURN(0);
}

EvalResult *evalInt(Value *ev, Node *p) {
  EVAL_HEADER;
  opStackPush(newIntValue((long)p->data));
  EVAL_RETURN(0);
}

EvalResult *evalList(Value *ev, Node *p) {
  EVAL_HEADER;
  List *vs = newList();
  opStackPush(newListValue(vs));
  int i, n = chldNum(p);
  for (i = 0; i < n; i++) {
    EvalResult *er = eval(ev, chld(p, i));
    if (er) {
      assert(er->type == EXCEPTION_RESULT);
      EVAL_RETURN(er);
    }
    listPush(vs, opStackPop());
  }
  EVAL_RETURN(0);
}

EvalResult *evalString(Value *ev, Node *p) {
  EVAL_HEADER;
  opStackPush(newStringValue(copyStr(p->data)));
  EVAL_RETURN(0);
}

EvalResult *evalNone(Value *ev, Node *p) {
  EVAL_HEADER;
  opStackPush(newNoneValue());
  EVAL_RETURN(0);
}

EvalResult *evalAdd(Value *ev, Node *p) {
  EVAL_HEADER;
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    EVAL_RETURN(er);
  }
  er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    EVAL_RETURN(er);
  }
  opStackPopNPush(2, valueAdd(opStackPeek(0), opStackPeek(1)));
  EVAL_RETURN(0);
}

EvalResult *evalSub(Value *ev, Node *p) {
  EVAL_HEADER;
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    EVAL_RETURN(er);
  }
  er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    EVAL_RETURN(er);
  }
  opStackPopNPush(2, valueSub(opStackPeek(0), opStackPeek(1)));
  EVAL_RETURN(0);
}

EvalResult *evalMul(Value *ev, Node *p) {
  EVAL_HEADER;
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    EVAL_RETURN(er);
  }
  er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    EVAL_RETURN(er);
  }
  opStackPopNPush(2, valueMul(opStackPeek(0), opStackPeek(1)));
  EVAL_RETURN(0);
}

EvalResult *evalDiv(Value *ev, Node *p) {
  EVAL_HEADER;
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    EVAL_RETURN(er);
  }
  er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    EVAL_RETURN(er);
  }
  opStackPopNPush(2, valueDiv(opStackPeek(0), opStackPeek(1)));
  EVAL_RETURN(0);
}

EvalResult *evalMod(Value *ev, Node *p) {
  EVAL_HEADER;
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    EVAL_RETURN(er);
  }
  er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    EVAL_RETURN(er);
  }
  opStackPopNPush(2, valueMod(opStackPeek(0), opStackPeek(1)));
  EVAL_RETURN(0);
}

EvalResult *evalGT(Value *ev, Node *p) {
  EVAL_HEADER;
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    EVAL_RETURN(er);
  }
  er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    EVAL_RETURN(er);
  }
  opStackPopNPush(2, newIntValue(valueCmp(opStackPeek(0), opStackPeek(1)) > 0));
  EVAL_RETURN(0);
}

EvalResult *evalLT(Value *ev, Node *p) {
  EVAL_HEADER;
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    EVAL_RETURN(er);
  }
  er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    EVAL_RETURN(er);
  }
  opStackPopNPush(2, newIntValue(valueCmp(opStackPeek(0), opStackPeek(1)) < 0));
  EVAL_RETURN(0);
}

EvalResult *evalGE(Value *ev, Node *p) {
  EVAL_HEADER;
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    EVAL_RETURN(er);
  }
  er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    EVAL_RETURN(er);
  }
  opStackPopNPush(2, newIntValue(valueCmp(opStackPeek(0), opStackPeek(1)) >= 0));
  EVAL_RETURN(0);
}

EvalResult *evalLE(Value *ev, Node *p) {
  EVAL_HEADER;
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    EVAL_RETURN(er);
  }
  er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    EVAL_RETURN(er);
  }
  opStackPopNPush(2, newIntValue(valueCmp(opStackPeek(0), opStackPeek(1)) <= 0));
  EVAL_RETURN(0);
}

EvalResult *evalEQ(Value *ev, Node *p) {
  EVAL_HEADER;
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    EVAL_RETURN(er);
  }
  er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    EVAL_RETURN(er);
  }
  opStackPopNPush(2, newIntValue(valueCmp(opStackPeek(0), opStackPeek(1)) == 0));
  EVAL_RETURN(0);
}

EvalResult *evalNE(Value *ev, Node *p) {
  EVAL_HEADER;
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    EVAL_RETURN(er);
  }
  er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    EVAL_RETURN(er);
  }
  opStackPopNPush(2, newIntValue(valueCmp(opStackPeek(0), opStackPeek(1)) != 0));
  EVAL_RETURN(0);
}

EvalResult *evalAnd(Value *ev, Node *p) {
  EVAL_HEADER;
  EvalResult *er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    EVAL_RETURN(er);
  }
  if (!(opStackPeek(0)->data)) {
    opStackPop();
    opStackPush(newIntValue(0));
    EVAL_RETURN(0);
  }
  er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    EVAL_RETURN(er);
  }
  opStackPopNPush(2, opStackPeek(0)->data ? newIntValue(1) : newIntValue(0));
  EVAL_RETURN(0);
}

EvalResult *evalOr(Value *ev, Node *p) {
  EVAL_HEADER;
  EvalResult *er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    EVAL_RETURN(er);
  }
  if (opStackPeek(0)->data) {
    opStackPop();
    opStackPush(newIntValue(1));
    EVAL_RETURN(0);
  }
  er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    EVAL_RETURN(er);
  }
  opStackPopNPush(2, opStackPeek(0)->data ? newIntValue(1) : newIntValue(0));
  EVAL_RETURN(0);
}

EvalResult *evalNot(Value *ev, Node *p) {
  EVAL_HEADER;
  EvalResult *er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    EVAL_RETURN(er);
  }
  opStackPopNPush(1, newIntValue(!(opStackPeek(0)->data)));
  EVAL_RETURN(0);
}

EvalResult *evalFun(Value *ev, Node *p) {
  EVAL_HEADER;
  opStackPush(newClosureValue(p, &ev));
  envPut(getEnvFromValue(ev), (long)chld(p, 0)->data, opStackPeek(0));
  EVAL_RETURN(0);
}

EvalResult *evalError(Value *ev, Node *p) {
  EVAL_HEADER;
  error("cannot eval unknown node type %d\n", p->type);
  EVAL_RETURN(0);
}

EvalResult *evalIf(Value *ev, Node *p) {
  EVAL_HEADER;
  EvalResult *er = eval(ev, chld(p, 0));
  if (er) EVAL_RETURN(er);
  if (opStackPop()->data) {
    er = eval(ev, chld(p, 1));
    if (er) EVAL_RETURN(er);
    EVAL_RETURN(0);
  } else {
    if (chldNum(p) == 3) {
      er = eval(ev, chld(p, 2));
      if (er) EVAL_RETURN(er);
      EVAL_RETURN(0);
    } else {
      opStackPush(newNoneValue());
      EVAL_RETURN(0);
    }
  }
}

EvalResult *evalExpList(Value *ev, Node *p) {
  EVAL_HEADER;
  int i, n = chldNum(p);
  for (i = 0; i < n; i++) {
    if (i) opStackPop();
    EvalResult *er = eval(ev, chld(p, i));
    if (er) EVAL_RETURN(er);
  }
  EVAL_RETURN(0);
}

EvalResult *evalListAccess(Value *ev, Node *p) {
  EVAL_HEADER;
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) EVAL_RETURN(er);
  er = eval(ev, chld(p, 0));
  if (er) {
    opStackPop();
    EVAL_RETURN(er);
  }
  long idx = getIntFromValue(opStackPeek(1));
  switch (opStackPeek(0)->type) {
    case LIST_VALUE_TYPE: {
      List *l = (List *)opStackPeek(0)->data;
      opStackPopNPush(2, listGet(l, idx));
      break;
    }
    case STRING_VALUE_TYPE: {
      char *s = (char *)opStackPeek(0)->data;
      char *ss = (char *)tlMalloc(2);
      ss[0] = s[idx];
      ss[1] = 0;
      opStackPopNPush(2, newStringValue(ss));
      break;
    }
    default:
      error("list access does not support value type %s\n", valueToString(opStackPeek(0)));
  }
  EVAL_RETURN(0);
}

EvalResult *evalTailRecursion(Value *ev, Node *p) {
  EVAL_HEADER;
  int i, n;
  List *l = newList();
  opStackPush(newListValue(l));  // protect from gc
  Node *args = chld(p, 1);
  n = chldNum(args);
  for (i = n - 1; i >= 0; i--) {
    EvalResult *er = eval(ev, chld(args, i));
    if (er) EVAL_RETURN(er);
    listPush(l, opStackPop());
  }
  EvalResult *er = eval(ev, chld(p, 0));
  if (er) EVAL_RETURN(er);
  listPush(l, opStackPop());
  EVAL_RETURN(newEvalResult(TAIL_RECURSION_RESULT, opStackPop()));
}

EvalResult *evalReturn(Value *ev, Node *p) {
  EVAL_HEADER;
  EvalResult *er = eval(ev, chld(p, 0));
  if (er) EVAL_RETURN(er);
  EVAL_RETURN(newEvalResult(RETURN_RESULT, opStackPop()));
}

EvalResult *evalAssign(Value *ev, Node *p) {
  EVAL_HEADER;
  int beforeStackSize = opStackSize();
  Node *left = chld(p, 0);
  int i;
  Env *e = ev->data;
  switch (left->type) {
    case LIST_ACCESS_TYPE: {
      EvalResult *er = eval(ev, chld(left, 0));
      if (er) {
        assert(er->type == EXCEPTION_RESULT);
        EVAL_RETURN(er);
      }
      assert(opStackPeek(0)->type == LIST_VALUE_TYPE);
      List *l = opStackPeek(0)->data;
      er = eval(ev, chld(left, 1));
      if (er) {
        assert(er->type == EXCEPTION_RESULT);
        opStackPopTo(beforeStackSize);
        EVAL_RETURN(er);
      }
      long idx = getIntFromValue(opStackPeek(0));
      er = eval(ev, chld(p, 1));
      if (er) {
        assert(er->type == EXCEPTION_RESULT);
        opStackPopTo(beforeStackSize);
        EVAL_RETURN(er);
      }
      opStackPopToPush(beforeStackSize, opStackPeek(0));
      listSet(l, idx, opStackPeek(0));
      EVAL_RETURN(0);
    }
    case ID_TYPE: {
      EvalResult *er = eval(ev, chld(p, 1));
      if (er) {
        assert(er->type == EXCEPTION_RESULT);
        EVAL_RETURN(er);
      }
      opStackPopNPush(1, opStackPeek(0));
      envPut(e, (long)left->data, opStackPeek(0));
      EVAL_RETURN(0);
    }
    case MODULE_ACCESS_TYPE: {
      Node *t = chld(p, 1);
      EvalResult *er = eval(ev, t);
      if (er) EVAL_RETURN(er);
      int n = chldNum(left);
      Env* curEnv = e;
      for (i = 0; i < n - 1; i++) {
        long id = (long)chld(left, i)->data;
        curEnv = getEnvFromValue(envGet(curEnv, id));
      }
      envPutLocal(curEnv, (long)chld(left, i)->data, opStackPeek(0));
      EVAL_RETURN(0);
    }
    default:
      error("left hand side of = must be a left value\n");
  }
  EVAL_RETURN(0); // never reach here
}

EvalResult *evalAddEq(Value *ev, Node *p) {
  EVAL_HEADER;
  int beforeStackSize = opStackSize();
  int i;
  Env *e = ev->data;
  Node *left = chld(p, 0);
  switch (left->type) {
    case LIST_ACCESS_TYPE: {
      EvalResult *er = eval(ev, chld(left, 0));
      if (er) {
        assert(er->type == EXCEPTION_RESULT);
        EVAL_RETURN(er);
      }
      List *l = getListFromValue(opStackPeek(0));

      er = eval(ev, chld(left, 1));
      if (er) {
        assert(er->type == EXCEPTION_RESULT);
        opStackPopTo(beforeStackSize);
        EVAL_RETURN(er);
      }
      long idx = getIntFromValue(opStackPeek(0));

      er = eval(ev, chld(p, 1));
      if (er) {
        assert(er->type == EXCEPTION_RESULT);
        opStackPopTo(beforeStackSize);
        EVAL_RETURN(er);
      }
      Value *e1 = listGet(l, idx);
      Value *e2 = opStackPeek(0);
      switch (e1->type) {
        case INT_VALUE_TYPE:
        case STRING_VALUE_TYPE:
          opStackPopToPush(beforeStackSize, valueAdd(e1, e2));
          listSet(l, idx, opStackPeek(0));
          break;
        case LIST_VALUE_TYPE:
          if (e2->type == LIST_VALUE_TYPE) {
            List *l = (List *)e2->data;
            for (i = 0; i < listSize(l); i++)
              listPush((List *)e1->data, listGet(l, i));
          } else {
            listPush((List *)e1->data, e2);
          }
          opStackPopToPush(beforeStackSize, e1);
          break;
        default:
          error("unknown type for operator +=: %d\n", e1->type);
      }
      EVAL_RETURN(0);
    }
    case ID_TYPE: {
      long key = (long)chld(p, 0)->data;
      EvalResult *er = eval(ev, chld(p, 1));
      if (er) {
        assert(er->type == EXCEPTION_RESULT);
        EVAL_RETURN(er);
      }
      Value *e1 = envGet(e, key);
      Value *e2 = opStackPeek(0);
      switch (e1->type) {
        case INT_VALUE_TYPE:
        case STRING_VALUE_TYPE:
          opStackPopToPush(beforeStackSize, valueAdd(e1, e2));
          envPut(e, key, opStackPeek(0));
          break;
        case LIST_VALUE_TYPE:
          if (e2->type == LIST_VALUE_TYPE) {
            List *l = (List *)e2->data;
            for (i = 0; i < listSize(l); i++)
              listPush((List *)e1->data, listGet(l, i));
          } else {
            listPush((List *)e1->data, e2);
          }
          opStackPopToPush(beforeStackSize, e1);
          break;
        default:
          error("unknown type for operator +=\n");
      }
      EVAL_RETURN(0);
    }
    default:
      error("left hand side of += must be left value\n");
  }
  EVAL_RETURN(0);  // never reach here
}

EvalResult *evalFor(Value *ev, Node *p) {
  EVAL_HEADER;
  EvalResult *er = eval(ev, chld(p, 0));
  if (er) EVAL_RETURN(er);
  opStackPop();
  while (1) {
    er = eval(ev, chld(p, 1));
    if (er) EVAL_RETURN(er);
    if (!(opStackPop()->data)) break;
    er = eval(ev, chld(p, 3));
    if (er) {
      if (CONTINUE_RESULT == er->type) {
        freeEvalResult(er);
        er = eval(ev, chld(p, 2));
        if (er) EVAL_RETURN(er);
        opStackPop();
        continue;
      } else if (BREAK_RESULT == er->type) {
        freeEvalResult(er);
        break;
      } else {
        EVAL_RETURN(er);
      }
    }
    opStackPop();
    er = eval(ev, chld(p, 2));
    if (er) EVAL_RETURN(er);
    opStackPop();
  }
  opStackPush(newNoneValue());
  EVAL_RETURN(0);
}

EvalResult *evalForEach(Value *ev, Node *p) {
  EVAL_HEADER;
  int i;
  Env *e = ev->data;
  Node *id = chld(p, 0);
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) EVAL_RETURN(er);
  List *l = getListFromValue(opStackPeek(0));
  int len = listSize(l);
  for (i = 0; i < len; i++) {
    envPut(e, (long)id->data, listGet(l, i));
    er = eval(ev, chld(p, 2));
    if (er) {
      if (CONTINUE_RESULT == er->type) {
        freeEvalResult(er);
        continue;
      } else if (BREAK_RESULT == er->type) {
        freeEvalResult(er);
        break;
      } else {
        opStackPop();
        EVAL_RETURN(er);
      }
    } else {
      opStackPop();
    }
  }
  opStackPopNPush(1, newNoneValue());
  EVAL_RETURN(0);
}

EvalResult *evalWhile(Value *ev, Node *p) {
  EVAL_HEADER;
  while (1) {
    EvalResult *er = eval(ev, chld(p, 0));
    if (er) EVAL_RETURN(er);
    if (!(opStackPop()->data)) break;
    er = eval(ev, chld(p, 1));
    if (er) {
      if (CONTINUE_RESULT == er->type) {
        freeEvalResult(er);
        continue;
      } else if (BREAK_RESULT == er->type) {
        freeEvalResult(er);
        break;
      } else {
        EVAL_RETURN(er);
      }
    } else {
      opStackPop();
    }
  }
  opStackPush(newNoneValue());
  EVAL_RETURN(0);
}

EvalResult *evalContinue(Value *ev, Node *p) {
  EVAL_HEADER;
  EVAL_RETURN(newEvalResult(CONTINUE_RESULT, 0));
}

EvalResult *evalBreak(Value *ev, Node *p) {
  EVAL_HEADER;
  EVAL_RETURN(newEvalResult(BREAK_RESULT, 0));
}

EvalResult *evalTime(Value *ev, Node *p) {
  clock_t st = clock();
  Node *t = chld(p, 0);
  EvalResult *er = eval(ev, t);
  if (er) EVAL_RETURN(er);
  fprintf(stderr, "time: %lf secs\n", (clock() - st) * 1.0 / CLOCKS_PER_SEC);
  EVAL_RETURN(0);
}

EvalResult *evalTry(Value *ev, Node *p) {
  EVAL_HEADER;
  int beforeStackSize = opStackSize();
  Env *e = ev->data;
  Node *tryBlock = chld(p, 0);
  long catchId = (long)chld(p, 1)->data;
  Node *catchBlock = chld(p, 2);
  Node *finallyBlock = chldNum(p) == 4 ? chld(p, 3) : 0;
  EvalResult *er = eval(ev, tryBlock);
  if (er) {
    if (EXCEPTION_RESULT == er->type) {
      envPutLocal(e, catchId, er->value);
      freeEvalResult(er);
      EvalResult *er2 = eval(ev, catchBlock);
      EvalResult *er3 = finallyBlock ? eval(ev, finallyBlock) : 0;
      if (er3) {
        if (er2) freeEvalResult(er2);
        opStackPopTo(beforeStackSize);
        EVAL_RETURN(er3);
      }
      if (er2) {
        opStackPopTo(beforeStackSize);
        EVAL_RETURN(er2);
      }
      opStackPopTo(beforeStackSize);
      opStackPush(newNoneValue());
      EVAL_RETURN(0);
    }
    EVAL_RETURN(er);
  }
  if (finallyBlock) {
    er = eval(ev, finallyBlock);
    if (er) {
      opStackPopTo(beforeStackSize);
      EVAL_RETURN(er);
    }
  }
  opStackPopTo(beforeStackSize);
  opStackPush(newNoneValue());
  EVAL_RETURN(0);
}

EvalResult *evalThrow(Value *ev, Node *p) {
  EVAL_HEADER;
  EvalResult *er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    EVAL_RETURN(er);
  }
  EVAL_RETURN(newEvalResult(EXCEPTION_RESULT, opStackPop()));
}

EvalResult *evalAddAdd(Value *ev, Node *p) {
  EVAL_HEADER;
  int beforeStackSize = opStackSize();
  Env *e = ev->data;
  Node *left = chld(p, 0);
  switch (left->type) {
    case ID_TYPE: {
      long id = (long)left->data;
      long newValue = getIntFromValue(envGet(e, id)) + 1;
      opStackPopToPush(beforeStackSize, envPut(e, id, newIntValue(newValue)));
      EVAL_RETURN(0);
    }
    case LIST_ACCESS_TYPE: {
      EvalResult *er = eval(ev, chld(left, 0));
      if (er) {
        assert(er->type == EXCEPTION_RESULT);
        opStackPopTo(beforeStackSize);
        EVAL_RETURN(er);
      }
      List *l = getListFromValue(opStackPeek(0));
      er = eval(ev, chld(left, 1));
      if (er) {
        opStackPopTo(beforeStackSize);
        EVAL_RETURN(er);
      }
      long idxv = getIntFromValue(opStackPeek(0));
      opStackPopToPush(beforeStackSize, listGet(l, idxv));
      long resultV = getIntFromValue(opStackPeek(0)) + 1;
      listSet(l, idxv, newIntValue(resultV));
      EVAL_RETURN(0);
    }
    default:
      error("++ does not support value type %s\n",
            nodeTypeToString(left->type));
  }
  EVAL_RETURN(0);  // never reach here
}

EvalResult *evalLocal(Value *ev, Node *p) {
  EVAL_HEADER;
  Node *ids = chld(p, 0);
  int i, n = chldNum(ids);
  Env *e = ev->data;
  for (i = 0; i < n; i++) {
    envPutLocal(e, (long)chld(ids, i)->data, newNoneValue());
  }
  opStackPush(newNoneValue());
  EVAL_RETURN(0);
}

EvalResult *evalImport(Value *ev, Node *p) {
  EVAL_HEADER;
  int i;
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

  opStackPush(newEnvValue(&globalEnv));
  EvalResult *er = eval(opStackPeek(0), listLast(parseTrees));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop(); // pop env
    EVAL_RETURN(er);
  }
  opStackPop(); // discard dummy eval result
  envPut(e, (long)id->data, opStackPeek(0));
  EVAL_RETURN(0);
}

EvalResult *evalModuleAccess(Value *ev, Node *p) {
  EVAL_HEADER;
  int i, n = chldNum(p);
  Env* e = getEnvFromValue(ev);
  for (i = 0; i < n; i++) {
    long id = getIdFromNode(chld(p, i));
    if (i != n-1) {
      e = getEnvFromValue(envGet(e, id));
    } else {
      opStackPush(envGet(e, id));
    }
  }
  EVAL_RETURN(0);
}

#undef EVAL_RETURN
#undef EVAL_HEADER
