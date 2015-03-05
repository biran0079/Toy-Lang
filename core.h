#ifndef _CORE_H_
#define _CORE_H_
#include "list.h"

typedef struct Token Token;
typedef struct Node Node;
typedef struct Value Value;
typedef struct Closure Closure;
typedef struct Env Env;
typedef struct EvalResult EvalResult;
typedef enum ValueType ValueType;
typedef enum NodeType NodeType;

void init(int argc, char **argv);
void cleanup();
long getIntId(char *s);
char *getStrId(long id);
void listCreatedObjectsCount();

extern int newNodeC, freeNodeC;
extern int newListC, freeListC;
extern int newHashTableC, freeHashTableC;
extern int newIntValueC, newStringValueC, newClosureValueC, newEnvValueC,
    newListValueC, newBuiltinFunC;
extern int freeIntValueC, freeStringValueC, freeClosureValueC, freeEnvValueC,
    freeListValueC, freeBuiltinFunC;
extern int newClosureC, freeClosureC;
extern int newEnvC, freeEnvC;
extern int memoryUsage;
extern int isInitialized;
#ifdef DEBUG_GC
extern List *astStack;
#endif

#define YYSTYPE Node *

#endif
