#include "ast.h"
#include "mem.h"
#include "core.h"
#include "env.h"
#include "builtinFun.h"
#include "gc.h"
#include "hashTable.h"
#include "value.h"
#include "util.h"
#include "eval.h"
#include "idMap.h"
#include "opStack.h"

List *parseTrees;  // make sure no tree is deleted during evaluation by keeping
                   // track of all parse trees
Env *globalEnv;
int memoryLimit = 64 * 1024 * 1024;
List *path;  // where import loads from
int isInitialized = 0;
int sysArgc;
char **sysArgv;

/*** ALL GLOBAL VARIABLES DECLARES ABOVE!  ***/

void listCreatedObjectsCount() {
  fprintf(stderr, "\tNode: %d (%d/%d)\n", newNodeC - freeNodeC, newNodeC,
          freeNodeC);
  fprintf(stderr, "\tIntValue: %d (%d/%d)\n", newIntValueC - freeIntValueC,
          newIntValueC, freeIntValueC);
  fprintf(stderr, "\tStringValue: %d (%d/%d)\n",
          newStringValueC - freeStringValueC, newStringValueC,
          freeStringValueC);
  fprintf(stderr, "\tClosureValue: %d (%d/%d)\n",
          newClosureValueC - freeClosureValueC, newClosureValueC,
          freeClosureValueC);
  fprintf(stderr, "\tListValue: %d (%d/%d)\n", newListValueC - freeListValueC,
          newListValueC, freeListValueC);
  fprintf(stderr, "\tClosure: %d (%d/%d)\n", newClosureC - freeClosureC,
          newClosureC, freeClosureC);
  fprintf(stderr, "\tEnv: %d (%d/%d)\n", newEnvC - freeEnvC, newEnvC, freeEnvC);
  fprintf(stderr, "\tList: %d (%d/%d)\n", newListC - freeListC, newListC,
          freeListC);
  fprintf(stderr, "\tHashTable: %d (%d/%d)\n", newHashTableC - freeHashTableC,
          newHashTableC, freeHashTableC);
  fprintf(stderr, "\tBuiltinFun: %d (%d/%d)\n",
          newBuiltinFunC - freeBuiltinFunC, newBuiltinFunC, freeBuiltinFunC);
  fprintf(stderr, "\tEvalResult: %d (%d/%d)\n",
          newEvalResultC - freeEvalResultC, newEvalResultC, freeEvalResultC);
  fprintf(stderr, "memory usage: %d\n", memoryUsage);
}

static void initPath(int argc, char** args) {
  path = newList();
  char *tlDir = getFolder(args[0]);
  listPush(path, catStr(tlDir, "lib/"));
  tlFree(tlDir);
  listPush(path, copyStr("./"));
  char *srcDir = getFolder(args[1]);
  if (strcmp(srcDir, "./")) {
    listPush(path, srcDir);
  }
}

static void cleanupPath() {
  int i, n = listSize(path);
  for (i = 0; i < n; i++) {
    tlFree(listGet(path, i));
  }
  freeList(path);
}

void init(int argc, char **args) {
  initMem();
  initPath(argc, args);
  initIdMap();
  initOpStack();

  // global env depends on value block (none value), id map (default 'this'
  // field)
  globalEnv = getEnvFromValue(newEnvValue(0));
  opStackPush(globalEnv->envValue);
  registerBuiltinFunctions(globalEnv);

  parseTrees = newList();

  isInitialized = 1;
}

void cleanup() {
  assert(globalEnv->envValue == opStackPop());  // make sure global env get GCed
  forceGC();
  cleanupOpStack();
  cleanupIdMap();
  cleanupPath();
  cleanupMem();
  int i, n = listSize(parseTrees);
  for (i = 0; i < n; i++) {
    freeNode(listGet(parseTrees, i));
  }
  isInitialized = 0;
}

