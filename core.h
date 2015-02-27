#ifndef _CORE_H_
#define _CORE_H_
#include "list.h"

typedef struct Token Token;
typedef struct Node Node;
typedef struct Value Value;
typedef struct Closure Closure;
typedef struct Env Env;
typedef enum ValueType ValueType;
typedef enum NodeType NodeType;
typedef void (*ValueFunc)(Value *);

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

#define YYSTYPE Node *

#endif
