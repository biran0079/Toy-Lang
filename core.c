#include "ast.h"
#include "core.h"
#include "env.h"
#include "tljmp.h"
#include "builtinFun.h"
#include "gc.h"
#include "hashTable.h"
#include "value.h"
#include "util.h"
#include "idMap.h"
#include "opStack.h"

List *parseTrees;
List *values;      // all values created
List *rootValues;  // all values should be treated as root when gc
Value *globalEnv;
int memoryLimit = 200000000;
List *path;  // where import loads from

int shouldDumpGCHistory = 0;
int gcTestMode = 0;
List *gcHistory;
int sysArgc;
char **sysArgv;

/*** ALL GLOBAL VARIABLES DECLARES ABOVE!  ***/

void listCreatedObjectsCount() {
  fprintf(stderr, "\tNode: %d %d\n", newNodeC, freeNodeC);
  fprintf(stderr, "\tIntValue: %d %d\n", newIntValueC, freeIntValueC);
  fprintf(stderr, "\tStringValue: %d %d\n", newStringValueC, freeStringValueC);
  fprintf(stderr, "\tClosureValue: %d %d\n", newClosureValueC,
          freeClosureValueC);
  fprintf(stderr, "\tListValue: %d %d\n", newListValueC, freeListValueC);
  fprintf(stderr, "\tClosure: %d %d\n", newClosureC, freeClosureC);
  fprintf(stderr, "\tEnv: %d %d\n", newEnvC, freeEnvC);
  fprintf(stderr, "\tList: %d %d\n", newListC, freeListC);
  fprintf(stderr, "\tHashTable: %d %d\n", newHashTableC, freeHashTableC);
  fprintf(stderr, "\tBuiltinFun: %d %d\n", newBuiltinFunC, freeBuiltinFunC);
}

void init(int argc, char **args) {
  initIdMap();
  initOpStack();
  initTljump();
  path = newList();
  char *tlDir = getFolder(args[0]);
  listPush(path, catStr(tlDir, "lib/"));
  tlFree(tlDir);
  parseTrees = newList();
  values = newList();
  rootValues = newList();
  gcHistory = newList();

  listPush(path, copyStr("./"));
  globalEnv = newEnvValue(newEnv(newNoneValue()));
  listPush(rootValues, globalEnv);
  registerBuiltinFunctions(globalEnv->data);
}

void cleanup() {
  cleanupIdMap();
  cleanupOpStack();
  cleanupTlJump();
  listClear(rootValues);
  forceGC();
  freeList(rootValues);
  freeList(values);
  int i, n = listSize(parseTrees);
  for (i = 0; i < n; i++) freeNode(listGet(parseTrees, i));
  n = listSize(path);
  for (i = 0; i < n; i++) tlFree(listGet(path, i));
  freeList(path);
  freeList(parseTrees);
  if (!shouldDumpGCHistory) clearGCHistory();
  tlFree(newNoneValue());
}

FILE *openFromPath(char *s, char *mode) {
  int i, n = listSize(path);
  FILE *f = 0;
  for (i = 0; i < n; i++) {
    char *p = listGet(path, i);
    char *fname = catStr(p, s);
    f = fopen(fname, mode);
    tlFree(fname);
    if (f) break;
  }
  return f;
}
