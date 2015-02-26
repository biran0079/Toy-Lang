#ifndef _EVAL_H_
#define _EVAL_H_
#include "core.h"
#include "value.h"
#include "builtinFun.h"

#ifndef USE_LEGACY_EVAL
typedef enum EvalResultType {
  CONTINUE_RESULT,
  BREAK_RESULT,
  EXCEPTION_RESULT,
  RETURN_RESULT,
  TAIL_RECURSION_RESULT,
} EvalResultType;

typedef struct EvalResult {
  EvalResultType type;
  Value* value;
} EvalResult;

EvalResult* newEvalResult(EvalResultType t, Value* v);

void freeEvalResult(EvalResult* er);

EvalResult* eval(Value* ev, Node *p);
EvalResult* evalStmts(Value *e, Node *p);
EvalResult* evalExpList(Value *ev, Node *p);
EvalResult* evalNone(Value *ev, Node *p);
EvalResult* evalList(Value *ev, Node *p);
EvalResult* evalListAccess(Value *ev, Node *p);
EvalResult* evalTailRecursion(Value *ev, Node *p);
EvalResult* evalCall(Value *ev, Node *p);
EvalResult* evalReturn(Value *ev, Node *p);
EvalResult* evalId(Value *ev, Node *p);
EvalResult* evalInt(Value *ev, Node *p);
EvalResult* evalString(Value *ev, Node *p);
EvalResult* evalAssign(Value *ev, Node *p);
EvalResult* evalAddEq(Value *ev, Node *p);
EvalResult* evalAdd(Value *ev, Node *p);
EvalResult* evalSub(Value *ev, Node *p);
EvalResult* evalMul(Value *ev, Node *p);
EvalResult* evalDiv(Value *ev, Node *p);
EvalResult* evalMod(Value *ev, Node *p);
EvalResult* evalIf(Value *ev, Node *p);
EvalResult* evalFor(Value *ev, Node *p);
EvalResult* evalForEach(Value *ev, Node *p);
EvalResult* evalWhile(Value *ev, Node *p);
EvalResult* evalContinue(Value *ev, Node *p);
EvalResult* evalBreak(Value *ev, Node *p);
EvalResult* evalGT(Value *ev, Node *p);
EvalResult* evalLT(Value *ev, Node *p);
EvalResult* evalGE(Value *ev, Node *p);
EvalResult* evalLE(Value *ev, Node *p);
EvalResult* evalEQ(Value *ev, Node *p);
EvalResult* evalNE(Value *ev, Node *p);
EvalResult* evalAnd(Value *ev, Node *p);
EvalResult* evalOr(Value *ev, Node *p);
EvalResult* evalNot(Value *ev, Node *p);
EvalResult* evalFun(Value *ev, Node *p);
EvalResult* evalTime(Value *ev, Node *p);
EvalResult* evalTry(Value *ev, Node *p);
EvalResult* evalThrow(Value *ev, Node *p);
EvalResult* evalAddAdd(Value *ev, Node *p);
EvalResult* evalLocal(Value *ev, Node *p);
EvalResult* evalImport(Value *ev, Node *p);
EvalResult* evalModuleAccess(Value *ev, Node *p);
EvalResult* evalError(Value *ev, Node *p);
#else 
Value* eval(Value* ev, Node *p);
Value *evalStmts(Value *e, Node *p);
Value *evalExpList(Value *ev, Node *p);
Value *evalNone(Value *ev, Node *p);
Value *evalList(Value *ev, Node *p);
Value *evalListAccess(Value *ev, Node *p);
Value *evalTailRecursion(Value *ev, Node *p);
Value *evalCall(Value *ev, Node *p);
Value *evalReturn(Value *ev, Node *p);
Value *evalId(Value *ev, Node *p);
Value *evalInt(Value *ev, Node *p);
Value *evalString(Value *ev, Node *p);
Value *evalAssign(Value *ev, Node *p);
Value *evalAddEq(Value *ev, Node *p);
Value *evalAdd(Value *ev, Node *p);
Value *evalSub(Value *ev, Node *p);
Value *evalMul(Value *ev, Node *p);
Value *evalDiv(Value *ev, Node *p);
Value *evalMod(Value *ev, Node *p);
Value *evalIf(Value *ev, Node *p);
Value *evalFor(Value *ev, Node *p);
Value *evalForEach(Value *ev, Node *p);
Value *evalWhile(Value *ev, Node *p);
Value *evalContinue(Value *ev, Node *p);
Value *evalBreak(Value *ev, Node *p);
Value *evalGT(Value *ev, Node *p);
Value *evalLT(Value *ev, Node *p);
Value *evalGE(Value *ev, Node *p);
Value *evalLE(Value *ev, Node *p);
Value *evalEQ(Value *ev, Node *p);
Value *evalNE(Value *ev, Node *p);
Value *evalAnd(Value *ev, Node *p);
Value *evalOr(Value *ev, Node *p);
Value *evalNot(Value *ev, Node *p);
Value *evalFun(Value *ev, Node *p);
Value *evalTime(Value *ev, Node *p);
Value *evalTry(Value *ev, Node *p);
Value *evalThrow(Value *ev, Node *p);
Value *evalAddAdd(Value *ev, Node *p);
Value *evalLocal(Value *ev, Node *p);
Value *evalImport(Value *ev, Node *p);
Value *evalModuleAccess(Value *ev, Node *p);
Value *evalError(Value *ev, Node *p);
void throwValue(Env *e, Value *v);
void pushRootValue(Value *v);

#endif

#endif
