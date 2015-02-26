#ifndef _EVAL_H_
#define _EVAL_H_
#include "core.h"
#include "value.h"
#include "builtinFun.h"

void throwValue(Env *e, Value *v);
void pushRootValue(Value *v);

#ifndef USE_LEGACY_EVAL
void eval(Value* ev, Node *p);
void evalStmts(Value *e, Node *p);
void evalExpList(Value *ev, Node *p);
void evalNone(Value *ev, Node *p);
void evalList(Value *ev, Node *p);
void evalListAccess(Value *ev, Node *p);
void evalTailRecursion(Value *ev, Node *p);
void evalCall(Value *ev, Node *p);
void evalReturn(Value *ev, Node *p);
void evalId(Value *ev, Node *p);
void evalInt(Value *ev, Node *p);
void evalString(Value *ev, Node *p);
void evalAssign(Value *ev, Node *p);
void evalAddEq(Value *ev, Node *p);
void evalAdd(Value *ev, Node *p);
void evalSub(Value *ev, Node *p);
void evalMul(Value *ev, Node *p);
void evalDiv(Value *ev, Node *p);
void evalMod(Value *ev, Node *p);
void evalIf(Value *ev, Node *p);
void evalFor(Value *ev, Node *p);
void evalForEach(Value *ev, Node *p);
void evalWhile(Value *ev, Node *p);
void evalContinue(Value *ev, Node *p);
void evalBreak(Value *ev, Node *p);
void evalGT(Value *ev, Node *p);
void evalLT(Value *ev, Node *p);
void evalGE(Value *ev, Node *p);
void evalLE(Value *ev, Node *p);
void evalEQ(Value *ev, Node *p);
void evalNE(Value *ev, Node *p);
void evalAnd(Value *ev, Node *p);
void evalOr(Value *ev, Node *p);
void evalNot(Value *ev, Node *p);
void evalFun(Value *ev, Node *p);
void evalTime(Value *ev, Node *p);
void evalTry(Value *ev, Node *p);
void evalThrow(Value *ev, Node *p);
void evalAddAdd(Value *ev, Node *p);
void evalLocal(Value *ev, Node *p);
void evalImport(Value *ev, Node *p);
void evalModuleAccess(Value *ev, Node *p);
void evalError(Value *ev, Node *p);
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
#endif

#endif
