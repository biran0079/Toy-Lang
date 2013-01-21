#include "execUnit.h"

ExecUnit* newExecUnit(Value* ev, Node* p){
  ExecUnit* res = MALLOC(ExecUnit);
  res->ev = ev;
  res->p = p;
  return res;
}

void freeExecUnit(ExecUnit* eu){
  free(eu);
}

Value* exec(ExecUnit* eu){
  return eval(eu->ev, eu->p);
}
