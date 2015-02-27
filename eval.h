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

EvalResult *eval(Value *ev, Node *p);
EvalResult *evalStmts(Value *e, Node *p);
EvalResult *evalExpList(Value *ev, Node *p);
EvalResult *evalNone(Value *ev, Node *p);
EvalResult *evalList(Value *ev, Node *p);
EvalResult *evalListAccess(Value *ev, Node *p);
EvalResult *evalTailRecursion(Value *ev, Node *p);
EvalResult *evalCall(Value *ev, Node *p);
EvalResult *evalReturn(Value *ev, Node *p);
EvalResult *evalId(Value *ev, Node *p);
EvalResult *evalInt(Value *ev, Node *p);
EvalResult *evalString(Value *ev, Node *p);
EvalResult *evalAssign(Value *ev, Node *p);
EvalResult *evalAddEq(Value *ev, Node *p);
EvalResult *evalAdd(Value *ev, Node *p);
EvalResult *evalSub(Value *ev, Node *p);
EvalResult *evalMul(Value *ev, Node *p);
EvalResult *evalDiv(Value *ev, Node *p);
EvalResult *evalMod(Value *ev, Node *p);
EvalResult *evalIf(Value *ev, Node *p);
EvalResult *evalFor(Value *ev, Node *p);
EvalResult *evalForEach(Value *ev, Node *p);
EvalResult *evalWhile(Value *ev, Node *p);
EvalResult *evalContinue(Value *ev, Node *p);
EvalResult *evalBreak(Value *ev, Node *p);
EvalResult *evalGT(Value *ev, Node *p);
EvalResult *evalLT(Value *ev, Node *p);
EvalResult *evalGE(Value *ev, Node *p);
EvalResult *evalLE(Value *ev, Node *p);
EvalResult *evalEQ(Value *ev, Node *p);
EvalResult *evalNE(Value *ev, Node *p);
EvalResult *evalAnd(Value *ev, Node *p);
EvalResult *evalOr(Value *ev, Node *p);
EvalResult *evalNot(Value *ev, Node *p);
EvalResult *evalFun(Value *ev, Node *p);
EvalResult *evalTime(Value *ev, Node *p);
EvalResult *evalTry(Value *ev, Node *p);
EvalResult *evalThrow(Value *ev, Node *p);
EvalResult *evalAddAdd(Value *ev, Node *p);
EvalResult *evalLocal(Value *ev, Node *p);
EvalResult *evalImport(Value *ev, Node *p);
EvalResult *evalModuleAccess(Value *ev, Node *p);
EvalResult *evalError(Value *ev, Node *p);

void initEval();
void cleanupEval();
void inStackPointerUpdateAddr(HashTable* addrMap);

#endif
