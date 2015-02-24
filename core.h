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
typedef void (*ValueFunc)(Value*);
typedef Value* (*BuiltinFun)(List* l); 

void init(int argc, char** argv);
void cleanup();
long getIntId(char *s);
char* getStrId(long id);

extern int newNodeC, freeNodeC;
extern int newListC, freeListC;
extern int newHashTableC, freeHashTableC;
extern int newIntValueC, newStringValueC, newClosureValueC, newEnvValueC, newListValueC, newBuiltinFunC;
extern int freeIntValueC, freeStringValueC, freeClosureValueC, freeEnvValueC, freeListValueC, freeBuiltinFunC;
extern int newClosureC, freeClosureC;
extern int newEnvC, freeEnvC;
extern int memoryUsage;

#define YYSTYPE Node*

#endif
