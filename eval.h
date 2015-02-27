#ifndef _EVAL_H_
#define _EVAL_H_
#include "core.h"
#include "value.h"
#include "builtinFun.h"
#include "hashTable.h"

typedef enum EvalResultType {
  CONTINUE_RESULT,
  BREAK_RESULT,
  EXCEPTION_RESULT,
  RETURN_RESULT,
  TAIL_RECURSION_RESULT,
} EvalResultType;

typedef struct EvalResult {
  EvalResultType type;
  Value *value;
} EvalResult;

int newEvalResultC, freeEvalResultC;

EvalResult *newEvalResult(EvalResultType t, Value *v);

void freeEvalResult(EvalResult *er);

EvalResult *eval(Env *ev, Node *p);
EvalResult *evalStmts(Env *e, Node *p);
EvalResult *evalExpList(Env *ev, Node *p);
EvalResult *evalNone(Env *ev, Node *p);
EvalResult *evalList(Env *ev, Node *p);
EvalResult *evalListAccess(Env *ev, Node *p);
EvalResult *evalTailRecursion(Env *ev, Node *p);
EvalResult *evalCall(Env *ev, Node *p);
EvalResult *evalReturn(Env *ev, Node *p);
EvalResult *evalId(Env *ev, Node *p);
EvalResult *evalInt(Env *ev, Node *p);
EvalResult *evalString(Env *ev, Node *p);
EvalResult *evalAssign(Env *ev, Node *p);
EvalResult *evalAddEq(Env *ev, Node *p);
EvalResult *evalAdd(Env *ev, Node *p);
EvalResult *evalSub(Env *ev, Node *p);
EvalResult *evalMul(Env *ev, Node *p);
EvalResult *evalDiv(Env *ev, Node *p);
EvalResult *evalMod(Env *ev, Node *p);
EvalResult *evalIf(Env *ev, Node *p);
EvalResult *evalFor(Env *ev, Node *p);
EvalResult *evalForEach(Env *ev, Node *p);
EvalResult *evalWhile(Env *ev, Node *p);
EvalResult *evalContinue(Env *ev, Node *p);
EvalResult *evalBreak(Env *ev, Node *p);
EvalResult *evalGT(Env *ev, Node *p);
EvalResult *evalLT(Env *ev, Node *p);
EvalResult *evalGE(Env *ev, Node *p);
EvalResult *evalLE(Env *ev, Node *p);
EvalResult *evalEQ(Env *ev, Node *p);
EvalResult *evalNE(Env *ev, Node *p);
EvalResult *evalAnd(Env *ev, Node *p);
EvalResult *evalOr(Env *ev, Node *p);
EvalResult *evalNot(Env *ev, Node *p);
EvalResult *evalFun(Env *ev, Node *p);
EvalResult *evalTime(Env *ev, Node *p);
EvalResult *evalTry(Env *ev, Node *p);
EvalResult *evalThrow(Env *ev, Node *p);
EvalResult *evalAddAdd(Env *ev, Node *p);
EvalResult *evalLocal(Env *ev, Node *p);
EvalResult *evalImport(Env *ev, Node *p);
EvalResult *evalModuleAccess(Env *ev, Node *p);
EvalResult *evalError(Env *ev, Node *p);

#endif
