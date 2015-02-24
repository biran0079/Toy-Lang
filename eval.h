#ifndef _EVAL_H_
#define _EVAL_H_
#include "core.h"
#include "value.h"
#include "builtinFun.h"

void throwValue(Env* e, Value* v);
void pushRootValue(Value* v);

Value* evalStmts(Value* e, Node* p);
Value* evalExpList(Value* ev, Node* p);
Value* evalNone(Value* ev, Node* p);
Value* evalList(Value* ev, Node* p);
Value* evalListAccess(Value* ev, Node* p);
Value* evalTailRecursion(Value* ev, Node* p);
Value* evalCall(Value* ev, Node* p);
Value* evalReturn(Value* ev, Node* p);
Value* evalId(Value* ev, Node* p);
Value* evalInt(Value* ev, Node* p);
Value* evalString(Value* ev, Node* p);
Value* evalAssign(Value* ev, Node* p);
Value* evalAddEq(Value* ev, Node* p);
Value* evalAdd(Value* ev, Node* p);
Value* evalSub(Value* ev, Node* p);
Value* evalMul(Value* ev, Node* p);
Value* evalDiv(Value* ev, Node* p);
Value* evalMod(Value* ev, Node* p);
Value* evalIf(Value* ev, Node* p);
Value* evalFor(Value* ev, Node* p);
Value* evalForEach(Value* ev, Node* p);
Value* evalWhile(Value* ev, Node* p);
Value* evalContinue(Value* ev, Node* p);
Value* evalBreak(Value* ev, Node* p);
Value* evalGT(Value* ev, Node* p);
Value* evalLT(Value* ev, Node* p);
Value* evalGE(Value* ev, Node* p);
Value* evalLE(Value* ev, Node* p);
Value* evalEQ(Value* ev, Node* p);
Value* evalNE(Value* ev, Node* p);
Value* evalAnd(Value* ev, Node* p);
Value* evalOr(Value* ev, Node* p);
Value* evalNot(Value* ev, Node* p);
Value* evalFun(Value* ev, Node* p);
Value* evalTime(Value* ev, Node* p);
Value* evalTry(Value* ev, Node* p);
Value* evalThrow(Value* ev, Node* p);
Value* evalAddAdd(Value* ev, Node* p);
Value* evalLocal(Value* ev, Node* p);
Value* evalImport(Value* ev, Node* p);
Value* evalModuleAccess(Value* ev, Node* p);
Value* evalError(Value* ev, Node* p);

#endif
