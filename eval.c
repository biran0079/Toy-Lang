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
extern Env *globalEnv;

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
 * 5. each opStackSave must be paired by exactly on opStackRestore before
 * returning
 * 6. any usage of Value* type in eval call shuold be pushed to
 * pointersInEvalStack to survive GC
 * 7. no GC in env operations and op-stack operations
 */
EvalResult *eval(Env *ev, Node *p) {
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

EvalResult *evalStmts(Env *ev, Node *p) {
  List *l = (List *)p->data;
  int i;
  for (i = 0; i < listSize(l); i++) {
    EvalResult *er = eval(ev, listGet(l, i));
    if (er) {
      return er;
    } else {
      opStackPop();
    }
  }
  opStackPush(newNoneValue());
  return 0;
}

EvalResult *evalCall(Env *ev, Node *p) {
  int i, n;
  Node *args = chld(p, 1);
  n = chldNum(args);
  int beforeStackSize = opStackSize();
  for (i = n - 1; i >= 0; i--) {
    EvalResult *er = eval(ev, chld(args, i));
    if (er) {
      assert(er->type == EXCEPTION_RESULT);
      opStackPopTo(beforeStackSize);
      return er;
    }
  }
  EvalResult *er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPopTo(beforeStackSize);
    return er;
  }
  while (1) {
    if (opStackPeek(0)->type == BUILTIN_FUN_VALUE_TYPE) {
      BuiltinFun f = opStackPeek(0)->data;
      opStackPop();  // pop closure
      f(n);
      // always success and result is in stack
      return 0;
    }
    // regular function call
    Closure *c = opStackPeek(0)->data;
    Node *f = c->f;

    Env *e2 = getEnvFromValue(newEnvValue(getEnvFromValue(c->e)));

    Node *ids = chld(f, 1);
    if (chldNum(ids) != n)
      error("%s parameter number incorrect\n", valueToString(opStackPeek(0)));

    for (i = 0; i < n; i++)
      envPutLocal(e2, (long)chld(ids, i)->data, opStackPeek(i + 1));

    opStackPopNPush(n + 1, e2->envValue);

    EvalResult *er = eval(getEnvFromValue(opStackPeek(0)), chld(f, 2));

    if (er) {
      switch (er->type) {
        case EXCEPTION_RESULT: {
          opStackPopTo(beforeStackSize);
          return er;
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
          return 0;
        }
        case CONTINUE_RESULT:
        case BREAK_RESULT:
          error("continue or break outside loop\n");
      }
    } else {
      // no return called, always return none
      opStackPopToPush(beforeStackSize, newNoneValue());
      return 0;
    }
  }
}

EvalResult *evalId(Env *ev, Node *p) {
  opStackPush(envGet(ev, (long)p->data));
  return 0;
}

EvalResult *evalInt(Env *ev, Node *p) {
  opStackPush(newIntValue((long)p->data));
  return 0;
}

EvalResult *evalList(Env *ev, Node *p) {
  List *vs = newList();
  opStackPush(newListValue(vs));
  int i, n = chldNum(p);
  for (i = 0; i < n; i++) {
    EvalResult *er = eval(ev, chld(p, i));
    if (er) {
      assert(er->type == EXCEPTION_RESULT);
      return er;
    }
    listPush(vs, opStackPop());
  }
  return 0;
}

EvalResult *evalString(Env *ev, Node *p) {
  opStackPush(newStringValue(copyStr(p->data)));
  return 0;
}

EvalResult *evalNone(Env *ev, Node *p) {
  opStackPush(newNoneValue());
  return 0;
}

EvalResult *evalAdd(Env *ev, Node *p) {
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    return er;
  }
  er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    return er;
  }
  opStackPopNPush(2, valueAdd(opStackPeek(0), opStackPeek(1)));
  return 0;
}

EvalResult *evalSub(Env *ev, Node *p) {
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    return er;
  }
  er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    return er;
  }
  opStackPopNPush(2, valueSub(opStackPeek(0), opStackPeek(1)));
  return 0;
}

EvalResult *evalMul(Env *ev, Node *p) {
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    return er;
  }
  er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    return er;
  }
  opStackPopNPush(2, valueMul(opStackPeek(0), opStackPeek(1)));
  return 0;
}

EvalResult *evalDiv(Env *ev, Node *p) {
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    return er;
  }
  er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    return er;
  }
  opStackPopNPush(2, valueDiv(opStackPeek(0), opStackPeek(1)));
  return 0;
}

EvalResult *evalMod(Env *ev, Node *p) {
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    return er;
  }
  er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    return er;
  }
  opStackPopNPush(2, valueMod(opStackPeek(0), opStackPeek(1)));
  return 0;
}

EvalResult *evalGT(Env *ev, Node *p) {
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    return er;
  }
  er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    return er;
  }
  opStackPopNPush(2, newIntValue(valueCmp(opStackPeek(0), opStackPeek(1)) > 0));
  return 0;
}

EvalResult *evalLT(Env *ev, Node *p) {
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    return er;
  }
  er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    return er;
  }
  opStackPopNPush(2, newIntValue(valueCmp(opStackPeek(0), opStackPeek(1)) < 0));
  return 0;
}

EvalResult *evalGE(Env *ev, Node *p) {
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    return er;
  }
  er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    return er;
  }
  opStackPopNPush(2,
                  newIntValue(valueCmp(opStackPeek(0), opStackPeek(1)) >= 0));
  return 0;
}

EvalResult *evalLE(Env *ev, Node *p) {
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    return er;
  }
  er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    return er;
  }
  opStackPopNPush(2,
                  newIntValue(valueCmp(opStackPeek(0), opStackPeek(1)) <= 0));
  return 0;
}

EvalResult *evalEQ(Env *ev, Node *p) {
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    return er;
  }
  er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    return er;
  }
  opStackPopNPush(2,
                  newIntValue(valueCmp(opStackPeek(0), opStackPeek(1)) == 0));
  return 0;
}

EvalResult *evalNE(Env *ev, Node *p) {
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    return er;
  }
  er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    return er;
  }
  opStackPopNPush(2,
                  newIntValue(valueCmp(opStackPeek(0), opStackPeek(1)) != 0));
  return 0;
}

EvalResult *evalAnd(Env *ev, Node *p) {
  EvalResult *er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    return er;
  }
  if (!(opStackPeek(0)->data)) {
    opStackPop();
    opStackPush(newIntValue(0));
    return 0;
  }
  er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    return er;
  }
  opStackPopNPush(2, opStackPeek(0)->data ? newIntValue(1) : newIntValue(0));
  return 0;
}

EvalResult *evalOr(Env *ev, Node *p) {
  EvalResult *er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    return er;
  }
  if (opStackPeek(0)->data) {
    opStackPop();
    opStackPush(newIntValue(1));
    return 0;
  }
  er = eval(ev, chld(p, 1));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();
    return er;
  }
  opStackPopNPush(2, opStackPeek(0)->data ? newIntValue(1) : newIntValue(0));
  return 0;
}

EvalResult *evalNot(Env *ev, Node *p) {
  EvalResult *er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    return er;
  }
  opStackPopNPush(1, newIntValue(!(opStackPeek(0)->data)));
  return 0;
}

EvalResult *evalFun(Env *ev, Node *p) {
  opStackPush(newClosureValue(p, ev));
  envPut(ev, (long)chld(p, 0)->data, opStackPeek(0));
  return 0;
}

EvalResult *evalError(Env *ev, Node *p) {
  error("cannot eval unknown node type %d\n", p->type);
  return 0;
}

EvalResult *evalIf(Env *ev, Node *p) {
  EvalResult *er = eval(ev, chld(p, 0));
  if (er) return er;
  if (opStackPop()->data) {
    er = eval(ev, chld(p, 1));
    if (er) return er;
    return 0;
  } else {
    if (chldNum(p) == 3) {
      er = eval(ev, chld(p, 2));
      if (er) return er;
      return 0;
    } else {
      opStackPush(newNoneValue());
      return 0;
    }
  }
}

EvalResult *evalExpList(Env *ev, Node *p) {
  int i, n = chldNum(p);
  for (i = 0; i < n; i++) {
    if (i) opStackPop();
    EvalResult *er = eval(ev, chld(p, i));
    if (er) return er;
  }
  return 0;
}

EvalResult *evalListAccess(Env *ev, Node *p) {
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) return er;
  er = eval(ev, chld(p, 0));
  if (er) {
    opStackPop();
    return er;
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
      error("list access does not support value type %s\n",
            valueToString(opStackPeek(0)));
  }
  return 0;
}

EvalResult *evalTailRecursion(Env *ev, Node *p) {
  int i, n;
  List *l = newList();
  opStackPush(newListValue(l));  // protect from gc
  Node *args = chld(p, 1);
  n = chldNum(args);
  for (i = n - 1; i >= 0; i--) {
    EvalResult *er = eval(ev, chld(args, i));
    if (er) return er;
    listPush(l, opStackPop());
  }
  EvalResult *er = eval(ev, chld(p, 0));
  if (er) return er;
  listPush(l, opStackPop());
  return newEvalResult(TAIL_RECURSION_RESULT, opStackPop());
}

EvalResult *evalReturn(Env *ev, Node *p) {
  EvalResult *er = eval(ev, chld(p, 0));
  if (er) return er;
  return newEvalResult(RETURN_RESULT, opStackPop());
}

EvalResult *evalAssign(Env *ev, Node *p) {
  int beforeStackSize = opStackSize();
  Node *left = chld(p, 0);
  int i;
  switch (left->type) {
    case LIST_ACCESS_TYPE: {
      EvalResult *er = eval(ev, chld(left, 0));
      if (er) {
        assert(er->type == EXCEPTION_RESULT);
        return er;
      }
      assert(opStackPeek(0)->type == LIST_VALUE_TYPE);
      List *l = opStackPeek(0)->data;
      er = eval(ev, chld(left, 1));
      if (er) {
        assert(er->type == EXCEPTION_RESULT);
        opStackPopTo(beforeStackSize);
        return er;
      }
      long idx = getIntFromValue(opStackPeek(0));
      er = eval(ev, chld(p, 1));
      if (er) {
        assert(er->type == EXCEPTION_RESULT);
        opStackPopTo(beforeStackSize);
        return er;
      }
      opStackPopToPush(beforeStackSize, opStackPeek(0));
      listSet(l, idx, opStackPeek(0));
      return 0;
    }
    case ID_TYPE: {
      EvalResult *er = eval(ev, chld(p, 1));
      if (er) {
        assert(er->type == EXCEPTION_RESULT);
        return er;
      }
      envPut(ev, (long)left->data, opStackPeek(0));
      return 0;
    }
    case MODULE_ACCESS_TYPE: {
      Node *t = chld(p, 1);
      EvalResult *er = eval(ev, t);
      if (er) return er;
      int n = chldNum(left);
      Env *curEnv = ev;
      for (i = 0; i < n - 1; i++) {
        long id = (long)chld(left, i)->data;
        curEnv = getEnvFromValue(envGet(curEnv, id));
      }
      envPutLocal(curEnv, (long)chld(left, i)->data, opStackPeek(0));
      return 0;
    }
    default:
      error("left hand side of = must be a left value\n");
  }
  return 0;  // never reach here
}

EvalResult *evalAddEq(Env *ev, Node *p) {
  int beforeStackSize = opStackSize();
  int i;
  Node *left = chld(p, 0);
  switch (left->type) {
    case LIST_ACCESS_TYPE: {
      EvalResult *er = eval(ev, chld(left, 0));
      if (er) {
        assert(er->type == EXCEPTION_RESULT);
        return er;
      }
      List *l = getListFromValue(opStackPeek(0));

      er = eval(ev, chld(left, 1));
      if (er) {
        assert(er->type == EXCEPTION_RESULT);
        opStackPopTo(beforeStackSize);
        return er;
      }
      long idx = getIntFromValue(opStackPeek(0));

      er = eval(ev, chld(p, 1));
      if (er) {
        assert(er->type == EXCEPTION_RESULT);
        opStackPopTo(beforeStackSize);
        return er;
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
      return 0;
    }
    case ID_TYPE: {
      long key = (long)chld(p, 0)->data;
      EvalResult *er = eval(ev, chld(p, 1));
      if (er) {
        assert(er->type == EXCEPTION_RESULT);
        return er;
      }
      Value *e1 = envGet(ev, key);
      Value *e2 = opStackPeek(0);
      switch (e1->type) {
        case INT_VALUE_TYPE:
        case STRING_VALUE_TYPE:
          opStackPopToPush(beforeStackSize, valueAdd(e1, e2));
          envPut(ev, key, opStackPeek(0));
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
      return 0;
    }
    default:
      error("left hand side of += must be left value\n");
  }
  return 0;  // never reach here
}

EvalResult *evalFor(Env *ev, Node *p) {
  EvalResult *er = eval(ev, chld(p, 0));
  if (er) return er;
  opStackPop();
  while (1) {
    er = eval(ev, chld(p, 1));
    if (er) return er;
    if (!(opStackPop()->data)) break;
    er = eval(ev, chld(p, 3));
    if (er) {
      if (CONTINUE_RESULT == er->type) {
        freeEvalResult(er);
        er = eval(ev, chld(p, 2));
        if (er) return er;
        opStackPop();
        continue;
      } else if (BREAK_RESULT == er->type) {
        freeEvalResult(er);
        break;
      } else {
        return er;
      }
    }
    opStackPop();
    er = eval(ev, chld(p, 2));
    if (er) return er;
    opStackPop();
  }
  opStackPush(newNoneValue());
  return 0;
}

EvalResult *evalForEach(Env *ev, Node *p) {
  int i;
  Node *id = chld(p, 0);
  EvalResult *er = eval(ev, chld(p, 1));
  if (er) return er;
  List *l = getListFromValue(opStackPeek(0));
  int len = listSize(l);
  for (i = 0; i < len; i++) {
    envPut(ev, (long)id->data, listGet(l, i));
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
        return er;
      }
    } else {
      opStackPop();
    }
  }
  opStackPopNPush(1, newNoneValue());
  return 0;
}

EvalResult *evalWhile(Env *ev, Node *p) {
  while (1) {
    EvalResult *er = eval(ev, chld(p, 0));
    if (er) return er;
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
        return er;
      }
    } else {
      opStackPop();
    }
  }
  opStackPush(newNoneValue());
  return 0;
}

EvalResult *evalContinue(Env *ev, Node *p) {
  return newEvalResult(CONTINUE_RESULT, 0);
}

EvalResult *evalBreak(Env *ev, Node *p) {
  return newEvalResult(BREAK_RESULT, 0);
}

EvalResult *evalTime(Env *ev, Node *p) {
  clock_t st = clock();
  Node *t = chld(p, 0);
  EvalResult *er = eval(ev, t);
  if (er) return er;
  fprintf(stderr, "time: %lf secs\n", (clock() - st) * 1.0 / CLOCKS_PER_SEC);
  return 0;
}

EvalResult *evalTry(Env *ev, Node *p) {
  int beforeStackSize = opStackSize();
  Node *tryBlock = chld(p, 0);
  long catchId = (long)chld(p, 1)->data;
  Node *catchBlock = chld(p, 2);
  Node *finallyBlock = chldNum(p) == 4 ? chld(p, 3) : 0;
  EvalResult *er = eval(ev, tryBlock);
  if (er) {
    if (EXCEPTION_RESULT == er->type) {
      envPutLocal(ev, catchId, er->value);
      freeEvalResult(er);
      EvalResult *er2 = eval(ev, catchBlock);
      EvalResult *er3 = finallyBlock ? eval(ev, finallyBlock) : 0;
      if (er3) {
        if (er2) freeEvalResult(er2);
        opStackPopTo(beforeStackSize);
        return er3;
      }
      if (er2) {
        opStackPopTo(beforeStackSize);
        return er2;
      }
      opStackPopTo(beforeStackSize);
      opStackPush(newNoneValue());
      return 0;
    }
    return er;
  }
  if (finallyBlock) {
    er = eval(ev, finallyBlock);
    if (er) {
      opStackPopTo(beforeStackSize);
      return er;
    }
  }
  opStackPopTo(beforeStackSize);
  opStackPush(newNoneValue());
  return 0;
}

EvalResult *evalThrow(Env *ev, Node *p) {
  EvalResult *er = eval(ev, chld(p, 0));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    return er;
  }
  return newEvalResult(EXCEPTION_RESULT, opStackPop());
}

EvalResult *evalAddAdd(Env *ev, Node *p) {
  int beforeStackSize = opStackSize();
  Node *left = chld(p, 0);
  switch (left->type) {
    case ID_TYPE: {
      long id = (long)left->data;
      long newValue = getIntFromValue(envGet(ev, id)) + 1;
      opStackPopToPush(beforeStackSize, envPut(ev, id, newIntValue(newValue)));
      return 0;
    }
    case LIST_ACCESS_TYPE: {
      EvalResult *er = eval(ev, chld(left, 0));
      if (er) {
        assert(er->type == EXCEPTION_RESULT);
        opStackPopTo(beforeStackSize);
        return er;
      }
      List *l = getListFromValue(opStackPeek(0));
      er = eval(ev, chld(left, 1));
      if (er) {
        opStackPopTo(beforeStackSize);
        return er;
      }
      long idxv = getIntFromValue(opStackPeek(0));
      opStackPopToPush(beforeStackSize, listGet(l, idxv));
      long resultV = getIntFromValue(opStackPeek(0)) + 1;
      listSet(l, idxv, newIntValue(resultV));
      return 0;
    }
    default:
      error("++ does not support value type %s\n",
            nodeTypeToString(left->type));
  }
  return 0;  // never reach here
}

EvalResult *evalLocal(Env *ev, Node *p) {
  Node *ids = chld(p, 0);
  int i, n = chldNum(ids);
  for (i = 0; i < n; i++) {
    envPutLocal(ev, (long)chld(ids, i)->data, newNoneValue());
  }
  opStackPush(newNoneValue());
  return 0;
}

EvalResult *evalImport(Env *ev, Node *p) {
  int i;
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

  opStackPush(newEnvValue(globalEnv));
  EvalResult *er = eval(getEnvFromValue(opStackPeek(0)), listLast(parseTrees));
  if (er) {
    assert(er->type == EXCEPTION_RESULT);
    opStackPop();  // pop env
    return er;
  }
  opStackPop();  // discard dummy eval result
  envPut(ev, (long)id->data, opStackPeek(0));
  return 0;
}

EvalResult *evalModuleAccess(Env *ev, Node *p) {
  int i, n = chldNum(p);
  for (i = 0; i < n; i++) {
    long id = getIdFromNode(chld(p, i));
    if (i != n - 1) {
      ev = getEnvFromValue(envGet(ev, id));
    } else {
      opStackPush(envGet(ev, id));
    }
  }
  return 0;
}
