#include "tl.h"
#include "env.h"
#include "tljmp.h"
#include "builtinFun.h"
#include "gc.h"
#include "util.h"

int newNodeC = 0, newIntValueC = 0, newStringValueC = 0, newClosureValueC = 0, newEnvValueC = 0,
    newListValueC = 0, newClosureC = 0, newEnvC = 0, newBuiltinFunC = 0;
int freeNodeC = 0, freeIntValueC = 0, freeStringValueC = 0, freeClosureValueC = 0, freeEnvValueC = 0,
    freeListValueC = 0, freeClosureC = 0, freeEnvC=0, freeBuiltinFunC = 0;
int newListC = 0, newHashTableC = 0, freeListC = 0, freeHashTableC = 0;

/*** ALL GLOBAL VARIABLES DECLARES BELOW!  ***/

Node* parseTree;
List* values;  // all values created
List* rootValues;  // all values should be treated as root when gc
Value* globalEnv;
JmpMsg __jmpMsg__;
int hardMemLimit = 1500000, softMemLimit = 1000000;

int shouldDumpGCHistory = 0;  
List* gcHistory;

/*** ALL GLOBAL VARIABLES DECLARES ABOVE!  ***/

void listCreatedObjectsCount() {
  fprintf(stderr, "\tNode: %d %d\n", newNodeC, freeNodeC);
  fprintf(stderr, "\tIntValue: %d %d\n", newIntValueC, freeIntValueC);
  fprintf(stderr, "\tStringValue: %d %d\n", newStringValueC, freeStringValueC);
  fprintf(stderr, "\tClosureValue: %d %d\n", newClosureValueC, freeClosureValueC);
  fprintf(stderr, "\tListValue: %d %d\n", newListValueC, freeListValueC);
  fprintf(stderr, "\tClosure: %d %d\n", newClosureC, freeClosureC);
  fprintf(stderr, "\tEnv: %d %d\n", newEnvC, freeEnvC);
  fprintf(stderr, "\tList: %d %d\n", newListC, freeListC);
  fprintf(stderr, "\tHashTable: %d %d\n", newHashTableC, freeHashTableC);
  fprintf(stderr, "\tBuitinFun: %d %d\n", newBuiltinFunC, freeBuiltinFunC);
}

void init() {
  values = newList();
  rootValues = newList();
  gcHistory = newList();
  globalEnv = newEnvValue(newEnv(newNoneValue()));
  listPush(rootValues, globalEnv);
  registerBuiltinFunctions();
}

void cleanup() {
  listClear(rootValues);
  forceGC();
  freeList(rootValues);
  freeList(values);
  freeNode(parseTree);
  if(!shouldDumpGCHistory) clearGCHistory();
}

