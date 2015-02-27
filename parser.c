#include "parser.h"
#include "util.h"
#include "builtinFun.h"
#include "eval.h"
#include "ast.h"

extern List *parseTrees;

static Node* predMatch(Node** p, Node* res) {
  if (*p) freeNode(*p);
  return *p = res;
}

void maybeFree(Node* p) {
  if (p) freeNode(p);
}

static Node *buildTree(Node *fst, List *rst) {
  int len = listSize(rst), i;
  for (i = len - 1; i >= 0; i--) {
    Node *n = listGet(rst, i);
    listSet(n->data, 0, fst);
    fst = n;
  }
  freeList(rst);
  return fst;
}

#define PM(n, pred) predMatch(&n, pred(t, ip))

Node *parse(List *tokens) {
  int idx = 0;
  Node *tree = stmts(tokens, &idx);
  if (idx < listSize(tokens)) {
    int i;
    for (i = 0; i < idx; i++) {
      printf("%s\n", tokenTypeToStr(((Token *)listGet(tokens, i))->type));
    }
    error("failed to parse the whole program.");
  }
  if (tree) {
    tree = postProcessAst(tree);
    listPush(parseTrees, tree);
    return tree;
  }
  return 0;
}

Node *stmts(List *t, int *ip) {
  Node *fst = 0, *rst = 0;
  if (PM(fst, stmt) && PM(rst, stmts)) {
    listPushFront(rst->data, fst);
    return rst;
  }
  return newNode2(STMTS_TYPE, 0);
}

static Node *tokenToNode(Token *token) {
  switch (token->type) {
    case INT_T:
      return newNode(INT_TYPE, token->data);
    case STRING_T:
      return newNode(STRING_TYPE, copyStr(token->data));
    case ID_T:
      return newNode(ID_TYPE, (void *)getIntId(token->data));
    case NONE_T:
      return newNode2(NONE_TYPE, 0);
    default:
      return 0;
  }
}

static Token* matchT(List* t, int* ip, Token_t type) {
  if (listSize(t) <= *ip) return 0;
  Token *token = (Token *)listGet(t, *ip);
  if (token->type == type) {
    (*ip)++;
    return token;
  }
  return 0;
}

static Node *matchM(List *t, int *ip, Token_t type) {
  Token* token = matchT(t, ip, type);
  return token ? tokenToNode(token) : 0;
}

static Node* tokenMatch(Node** p, List* t, int *ip, Token_t type) {
  if (*p) freeNode(*p);
  return *p = matchM(t, ip, type);
}

#define TM(n, type) tokenMatch(&n, t, ip, type)
#define M(TYPE) matchT(t, ip, TYPE)
#define M2(T1, T2) M(T1) && M(T2)

Node *stmt(List *t, int *ip) {
  Node *n1 = 0, *n2 = 0, *n3 = 0, *n4 = 0;
  int i0 = *ip;
  if (M(RETURN_T) && M(SEMICOLON_T)) {
    return newNode2(RETURN_TYPE, 1, newNode2(NONE_TYPE, 0));
  } else if ((*ip = i0), M2(CONTINUE_T, SEMICOLON_T)) {
    return newNode2(CONTINUE_TYPE, 0);
  } else if ((*ip = i0), M2(BREAK_T, SEMICOLON_T)) {
    return newNode2(BREAK_TYPE, 0);
  } else if ((*ip = i0), PM(n1, expr) && M(SEMICOLON_T)) {
    return n1;
  } else if ((*ip = i0), M(IMPORT_T) && TM(n1, ID_T) && M(SEMICOLON_T)) {
    return newNode2(IMPORT_TYPE, 1, n1);
  } else if ((*ip = i0), M(LOCAL_T) && PM(n1, idList) && M(SEMICOLON_T)) {
    return newNode2(LOCAL_TYPE, 1, n1);
  } else if ((*ip = i0), M(THROW_T) && PM(n1, expr) && M(SEMICOLON_T)) {
    return newNode2(THROW_TYPE, 1, n1);
  } else if ((*ip = i0), M(RETURN_T) && PM(n1, expr) && M(SEMICOLON_T)) {
    return newNode2(RETURN_TYPE, 1, n1);
  } else if ((*ip = i0), M2(IF_T, OP_B_T) && PM(n1, expr) && M(CLO_B_T) &&
                             PM(n2, block)) {
    int i1 = *ip;
    if (M(ELSE_T) && PM(n3, block)) {
      return newNode2(IF_TYPE, 3, n1, n2, n3);
    } else {
      *ip = i1;
      return newNode2(IF_TYPE, 2, n1, n2);
    }
  } else if ((*ip = i0), M2(WHILE_T, OP_B_T) && PM(n1, expr) &&
                             M(CLO_B_T) && PM(n2, block)) {
    return newNode2(WHILE_TYPE, 2, n1, n2);
  } else if ((*ip = i0), M2(FOR_T, OP_B_T) && TM(n1, ID_T) && M(COLON_T) &&
                             PM(n2, expr) && M(CLO_B_T) &&
                             PM(n3, block)) {
    return newNode2(FOREACH_TYPE, 3, n1, n2, n3);
  } else if ((*ip = i0), M(FUN_T) && TM(n1, ID_T) && M(OP_B_T) &&
                             PM(n2, idList) && M(CLO_B_T) && M(OP_CB_T) &&
                             PM(n3, stmts) && M(CLO_CB_T)) {
    markTailRecursions(n3);
    return newNode2(FUN_TYPE, 3, n1, n2, n3);
  } else if ((*ip = i0), M(TRY_T) && PM(n1, block) && M(CATCH_T) &&
                             M(OP_B_T) && TM(n2, ID_T) && M(CLO_B_T) &&
                             PM(n3, block)) {
    int i1 = *ip;
    if (M(FINALLY_T) && PM(n4, block)) {
      return newNode2(TRY_TYPE, 4, n1, n2, n3, n4);
    } else {
      *ip = i1;
      return newNode2(TRY_TYPE, 3, n1, n2, n3);
    }
  } else if ((*ip = i0), M2(FOR_T, OP_B_T) && PM(n1, expList) &&
                             M(SEMICOLON_T) && PM(n2, expList) &&
                             M(SEMICOLON_T) && PM(n3, expList) &&
                             M(CLO_B_T) && PM(n4, block)) {
    return newNode2(FOR_TYPE, 4, n1, n2, n3, n4);
  }
  maybeFree(n1);
  maybeFree(n2);
  maybeFree(n3);
  maybeFree(n4);
  return 0;
}

Node *idList(List *t, int *ip) {
  int i0 = *ip;
  Node *n = 0;
  if (PM(n, nonEmptyIdList)) {
    return n;
  } else {
    *ip = i0;
    return newNode2(ID_LIST_TYPE, 0);
  }
}

Node *nonEmptyIdList(List *t, int *ip) {
  int i0 = *ip;
  Node *n1 = 0, *n2 = 0;
  if (TM(n1, ID_T) && M(COMMA_T) && PM(n2, nonEmptyIdList)) {
    listPushFront(n2->data, n1);
    return n2;
  } else if ((*ip = i0), TM(n1, ID_T)) {
    return newNode2(ID_LIST_TYPE, 1, n1);
  }
  maybeFree(n1);
  maybeFree(n2);
  return 0;
}

// module access
Node *moduleAccess(List *t, int *ip) {
  int i0 = *ip;
  Node *n1 = 0, *n2 = 0;
  if (TM(n1, ID_T) && M(DOT_T) && PM(n2, moduleAccess)) {
    listPushFront(n2->data, n1);
    return n2;
  } else if ((*ip = i0), TM(n1, ID_T) && M(DOT_T) && TM(n2, ID_T)) {
    return newNode2(MODULE_ACCESS_TYPE, 2, n1, n2);
  }
  maybeFree(n1);
  maybeFree(n2);
  return 0;
}

Node *listExpr(List *t, int *ip) {
  Node *n = 0;
  if (M(OP_SB_T) && PM(n, expList) && M(CLO_SB_T)) {
    n->type = LIST_TYPE;
    return n;
  }
  maybeFree(n);
  return 0;
}

List *listAccessOrCallInternal(List *t, int *ip) {
  int i0 = *ip;
  Node *n = 0;
  List *l;
  if (M(OP_SB_T) && PM(n, expr) && M(CLO_SB_T)) {
    n = newNode2(LIST_ACCESS_TYPE, 2, (void *)0, n);
    l = listAccessOrCallInternal(t, ip);
    listPush(l, n);
    return l;
  } else if ((*ip = i0), M(OP_B_T) && PM(n, expList) && M(CLO_B_T)) {
    n = newNode2(CALL_TYPE, 2, (void *)0, n);
    l = listAccessOrCallInternal(t, ip);
    listPush(l, n);
    return l;
  } else if ((*ip = i0), M(ADDADD_T)) {
    n = newNode2(ADDADD_TYPE, 1, (void *)0);
    // no [] and () can follow ++
    l = newList();
    listPush(l, n);
    return l;
  }
  // Restore *ip value if no matching found.
  // This is different from normal production matcher, because there is no
  // "maching failure".
  *ip = i0;
  return newList();
}

// id, string, int literal, list literal
Node *expr0(List *t, int *ip) {
  int i0 = *ip;
  Node *n = 0;
  List *l;
  if (TM(n, INT_T)) {
    return n;
  } else if ((*ip = i0), TM(n, NONE_T)) {
    return n;
  } else if ((*ip = i0), M(SUB_T) && TM(n, INT_T)) {
    n->data = (void *)(-(long)n->data);
    return n;
  } else if ((*ip = i0), M(OP_B_T) && PM(n, expr) && M(CLO_B_T)) {
    return buildTree(n, listAccessOrCallInternal(t, ip));
  } else if ((*ip = i0), PM(n, moduleAccess)) {
    return buildTree(n, listAccessOrCallInternal(t, ip));
  } else if ((*ip = i0), TM(n, ID_T)) {
    return buildTree(n, listAccessOrCallInternal(t, ip));
  } else if ((*ip = i0), TM(n, STRING_T)) {
    return buildTree(n, listAccessOrCallInternal(t, ip));
  } else if ((*ip = i0), PM(n, listExpr)) {
    return buildTree(n, listAccessOrCallInternal(t, ip));
  }
  maybeFree(n);
  return 0;
}

List *mulDivModInternal(List *t, int *ip) {
  int i0 = *ip;
  List *l;
  Node *n = 0;
  if (M(MUL_T) && PM(n, expr0)) {
    l = mulDivModInternal(t, ip);
    listPush(l, newNode2(MUL_TYPE, 2, 0, n));
    return l;
  } else if ((*ip = i0), M(DIV_T) && PM(n, expr0)) {
    l = mulDivModInternal(t, ip);
    listPush(l, newNode2(DIV_TYPE, 2, 0, n));
    return l;
  } else if ((*ip = i0), M(MOD_T) && PM(n, expr0)) {
    l = mulDivModInternal(t, ip);
    listPush(l, newNode2(MOD_TYPE, 2, 0, n));
    return l;
  }
  *ip = i0;
  return newList();
}

// * / %
Node *expr1(List *t, int *ip) {
  Node *n = 0;
  if (PM(n, expr0)) {
    return buildTree(n, mulDivModInternal(t, ip));
  }
  maybeFree(n);
  return 0;
}

List *addSubInternal(List *t, int *ip) {
  int i0 = *ip;
  List *l;
  Node *n = 0;
  if (M(ADD_T) && PM(n, expr1)) {
    l = addSubInternal(t, ip);
    listPush(l, newNode2(ADD_TYPE, 2, 0, n));
    return l;
  } else if ((*ip = i0), M(SUB_T) && PM(n, expr1)) {
    l = addSubInternal(t, ip);
    listPush(l, newNode2(SUB_TYPE, 2, 0, n));
    return l;
  }
  *ip = i0;
  return newList();
}
// + -
Node *expr2(List *t, int *ip) {
  Node *n = 0;
  if (PM(n, expr1)) {
    return buildTree(n, addSubInternal(t, ip));
  }
  maybeFree(n);
  return 0;
}

// = +=
Node *assignmentExpr(List *t, int *ip) {
  Node *n1 = 0, *n2 = 0;
  if (PM(n1, expr0)) {
    int i0 = *ip;
    if (M(ASSIGN_T) && PM(n2, expr)) {
      return newNode2(ASSIGN_TYPE, 2, n1, n2);
    } else if ((*ip = i0), M(ADDEQ_T) && PM(n2, expr)) {
      return newNode2(ADDEQ_TYPE, 2, n1, n2);
    }
  }
  maybeFree(n1);
  maybeFree(n2);
  return 0;
}

Node *expr3(List *t, int *ip) {
  Node *n1 = 0, *n2 = 0;
  int i0 = *ip;
  if (M(NOT_T) && PM(n1, expr3)) {
    return newNode2(NOT_TYPE, 1, n1);
  } else if ((*ip = i0), PM(n1, expr2)) {
    int i1 = *ip;
    if (M(EQ_T) && PM(n2, expr2)) {
      return newNode2(EQ_TYPE, 2, n1, n2);
    } else if ((*ip = i1), M(NE_T) && PM(n2, expr2)) {
      return newNode2(NE_TYPE, 2, n1, n2);
    } else if ((*ip = i1), M(GE_T) && PM(n2, expr2)) {
      return newNode2(GE_TYPE, 2, n1, n2);
    } else if ((*ip = i1), M(LE_T) && PM(n2, expr2)) {
      return newNode2(LE_TYPE, 2, n1, n2);
    } else if ((*ip = i1), M(GT_T) && PM(n2, expr2)) {
      return newNode2(GT_TYPE, 2, n1, n2);
    } else if ((*ip = i1), M(LT_T) && PM(n2, expr2)) {
      return newNode2(LT_TYPE, 2, n1, n2);
    }
    *ip = i1;
    return n1;
  }
  maybeFree(n1);
  maybeFree(n2);
  return 0;
}

List *andInternal(List *t, int *ip) {
  int i0 = *ip;
  List *l;
  Node *n = 0;
  if (M(AND_T) && PM(n, expr3)) {
    l = andInternal(t, ip);
    listPush(l, newNode2(AND_TYPE, 2, 0, n));
    return l;
  }
  *ip = i0;
  return newList();
}

Node *expr4(List *t, int *ip) {
  Node *n = 0;
  if (PM(n, expr3)) {
    return buildTree(n, andInternal(t, ip));
  }
  maybeFree(n);
  return 0;
}

List *orInternal(List *t, int *ip) {
  int i0 = *ip;
  List *l;
  Node *n = 0;
  if (M(OR_T) && PM(n, expr4)) {
    l = orInternal(t, ip);
    listPush(l, newNode2(OR_TYPE, 2, 0, n));
    return l;
  }
  *ip = i0;
  return newList();
}

Node *expr5(List *t, int *ip) {
  Node *n = 0;
  if (PM(n, expr4)) {
    return buildTree(n, orInternal(t, ip));
  }
  maybeFree(n);
  return 0;
}

Node *lambdaExpr(List *t, int *ip) {
  Node *n1 = 0, *n2 = 0;
  if (M2(LAMBDA_T, OP_B_T) && PM(n1, idList) && M2(CLO_B_T, OP_CB_T) &&
      PM(n2, stmts) && M(CLO_CB_T)) {
    return newNode2(FUN_TYPE, 3, newNode(ID_TYPE, (void *)getIntId("lambda")),
                    n1, n2);
  }
  maybeFree(n1);
  maybeFree(n2);
  return 0;
}

Node *timeExpr(List *t, int *ip) {
  Node *n = 0;
  if (M2(TIME_T, OP_B_T) && PM(n, expr) && M(CLO_B_T)) {
    return newNode2(TIME_TYPE, 1, n);
  }
  maybeFree(n);
  return 0;
}

Node *expr(List *t, int *ip) {
  int i0 = *ip;
  Node *n = 0;
  if (PM(n, assignmentExpr)) {
    return n;
  } else if ((*ip = i0), PM(n, lambdaExpr)) {
    return n;
  } else if ((*ip = i0), PM(n, timeExpr)) {
    return n;
  }
  *ip = i0;
  return expr5(t, ip);
}

Node *block(List *t, int *ip) {
  if (listSize(t) <= *ip) return 0;
  int i0 = *ip;
  Node *n = 0;
  if (M(OP_CB_T) && PM(n, stmts) && M(CLO_CB_T)) {
    return n;
  } else if ((*ip = i0), PM(n, stmt)) {
    return n;
  }
  maybeFree(n);
  return 0;
}

Node *expList(List *t, int *ip) {
  if (listSize(t) < *ip) return 0;
  int i0 = *ip;
  Node *n = 0;
  if (PM(n, nonEmptyExpList)) {
    return n;
  } else {
    *ip = i0;
    return newNode2(EXP_LIST_TYPE, 0);
  }
}

Node *nonEmptyExpList(List *t, int *ip) {
  if (listSize(t) <= *ip) return 0;
  int i0 = *ip;
  Node *n1 = 0, *n2 = 0;
  if (PM(n1, expr) && M(COMMA_T) && PM(n2, nonEmptyExpList)) {
    listPushFront(n2->data, n1);
    return n2;
  } else if ((*ip = i0), n1 = expr(t, ip)) {
    return newNode2(EXP_LIST_TYPE, 1, n1);
  }
  maybeFree(n1);
  maybeFree(n2);
  return 0;
}

#undef M
#undef M2
