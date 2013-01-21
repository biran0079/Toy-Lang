#ifndef __EXEC_H__
#define __EXEC_H__

#include "eval.h"
#include "util.h"

typedef struct ExecUnit {
  Value* ev;
  Node* p;
} ExecUnit;

ExecUnit* newExecUnit(Value* ev, Node* p);
void freeExecUnit(ExecUnit* eu);
Value* exec(ExecUnit* eu);

#endif
