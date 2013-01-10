#ifndef _TL_H_
#define _TL_H_

typedef struct Node Node;
typedef struct Value Value;
typedef struct Closure Closure;
typedef struct Env Env;
typedef enum ValueType ValueType;
typedef enum NodeType NodeType;
typedef void (*ValueFunc)(Value*);

void init();

#define YYSTYPE Node*

#endif
