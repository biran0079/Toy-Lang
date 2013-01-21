#ifndef _TL_H_
#define _TL_H_
#include "list.h"

typedef struct Node Node;
typedef struct Value Value;
typedef struct Closure Closure;
typedef struct Env Env;
typedef enum ValueType ValueType;
typedef enum NodeType NodeType;
typedef void (*ValueFunc)(Value*);
typedef Value* (*BuiltinFun)(List* l); 

void init();
void cleanup();
long getIntId(char *s);
char* getStrId(long id);

#define YYSTYPE Node*

#endif
