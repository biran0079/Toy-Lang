#ifndef _AST_H_
#define _AST_H_
#include "core.h"
#include "list.h"
#include "env.h"
#include "eval.h"
#define chldNum(t) listSize((List *)(t)->data)
#define chld(t, i) ((Node *)listGet((List *)(t)->data, i))

enum NodeType {
  INT_TYPE,
  ID_TYPE,
  STMTS_TYPE,
  ASSIGN_TYPE,
  ADD_TYPE,
  SUB_TYPE,
  MUL_TYPE,
  DIV_TYPE,
  IF_TYPE,
  GT_TYPE,
  LT_TYPE,
  GE_TYPE,
  LE_TYPE,
  EQ_TYPE,
  NE_TYPE,
  AND_TYPE,
  OR_TYPE,
  NOT_TYPE,
  WHILE_TYPE,
  FUN_TYPE,
  EXP_LIST_TYPE,
  ID_LIST_TYPE,
  CALL_TYPE,
  TAIL_CALL_TYPE,
  RETURN_TYPE,
  BREAK_TYPE,
  CONTINUE_TYPE,
  MOD_TYPE,
  LIST_TYPE,
  LIST_ACCESS_TYPE,
  FOR_TYPE,
  ADDEQ_TYPE,
  TIME_TYPE,
  STRING_TYPE,  // liter string
  NONE_TYPE,
  FOREACH_TYPE,
  TRY_TYPE,
  THROW_TYPE,
  ADDADD_TYPE,
  LOCAL_TYPE,
  IMPORT_TYPE,
  MODULE_ACCESS_TYPE,
  __DUMMY_TYPE,  // Used for parsing. Not part of language.
};

#ifndef USE_LEGACY_EVAL
typedef EvalResult *(*EvalFunc)(Value *, Node *);
#else
typedef Value *(*EvalFunc)(Value *, Node *);
#endif

struct Node {
  NodeType type;
  void *data;
  EvalFunc eval;
};

Node *newNode(NodeType type, void *data);
Node *newNode2(NodeType type, int n, ...);

void freeNode(Node *t);

void markTailRecursions(Node *t);

char *nodeTypeToString(NodeType type);

Value *nodeToListValue(Node *p);

void printAst(Node *ast);

#endif
