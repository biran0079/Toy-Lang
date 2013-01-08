#include "tl.h"
#include "env.h"
#include "util.h"

int newNodeC = 0, newNode2C = 0, newIntValueC = 0, newStringValueC = 0, newClosureValueC = 0, 
    newListValueC = 0, newClosureC = 0, newEnvC = 0;
int freeNodeC = 0, freeNode2C = 0, freeIntValueC = 0, freeStringValueC = 0, freeClosureValueC = 0, 
    freeListValueC = 0, freeClosureC = 0, freeEnvC=0;
int newListC = 0, newHashTableC = 0, freeListC = 0, freeHashTableC = 0;

void listCreatedObjectsCount(){
  fprintf(stderr, "\tNode: %d %d\n", newNodeC, freeNodeC);
  fprintf(stderr, "\tNode2: %d %d\n", newNode2C, freeNode2C);
  fprintf(stderr, "\tIntValue: %d %d\n", newIntValueC, freeIntValueC);
  fprintf(stderr, "\tStringValue: %d %d\n", newStringValueC, freeStringValueC);
  fprintf(stderr, "\tFunValue: %d %d\n", newClosureValueC, freeClosureValueC);
  fprintf(stderr, "\tListValue: %d %d\n", newListValueC, freeListValueC);
  fprintf(stderr, "\tClosure: %d %d\n", newClosureC, freeClosureC);
  fprintf(stderr, "\tEnv: %d %d\n", newEnvC, freeEnvC);
  fprintf(stderr, "\tList: %d %d\n", newListC, freeListC);
  fprintf(stderr, "\tHashTable: %d %d\n", newHashTableC, freeHashTableC);
}

void throwValue(Env* e, Value* v) {
  e->exceptionValue = v;  // pass exception value through enviconment
  if(listSize(e->exceptionStates)) {
    // currently in try block
    longjmp(listLast(e->exceptionStates), 1);
  } else {
    // If has parent stack frame, pass to it. Report error otherwise
    if (e->parent) {
      longjmp(e->retState, 3);
    } else {
      error("uncaught exception:\n%s\n", valueToString(v));
    }
  }
  error("should never reach here\n");
}

