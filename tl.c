#include "tl.h"
#include "env.h"
#include "util.h"

int newNodeC = 0, newNode2C = 0, newIntValueC = 0, newStringValueC = 0, newClosureValueC = 0, newEnvValueC = 0,
    newListValueC = 0, newClosureC = 0, newEnvC = 0;
int freeNodeC = 0, freeNode2C = 0, freeIntValueC = 0, freeStringValueC = 0, freeClosureValueC = 0, freeEnvValueC = 0,
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

