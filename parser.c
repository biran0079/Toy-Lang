#include "parser.h"
#include "util.h"
#include "builtinFun.h"
#include "value.h"

extern List* parseTrees;

Node* parse(List* tokens) {
  int idx = 0;;
  Node* tree = stmts(tokens, &idx);
  if (idx < listSize(tokens)) {
    int i;
    for (i = 0; i < idx; i++) {
      printf("%s\n", tokenTypeToStr(((Token*) listGet(tokens, i))->type));
    }
    error("failed to parse the whole program.");
  }
  if (tree) {
    listPush(parseTrees, tree);
  }
  return tree;
}

Node* stmts(List* t, int* ip) {
  Node* fst, *rst;
  if ((fst = stmt(t, ip)) && (rst = stmts(t, ip))) {
    listPushFront(rst->data, fst);
    return rst;
  }
  return newNode2(STMTS_TYPE, 0);
}

Node* tokenToNode(Token* token) {
  static Node* dummyNode = 0;
  switch (token->type) {
    case INT_T: return newNode(INT_TYPE, token->data);
    case STRING_T: return newNode(STRING_TYPE, token->data);
    case ID_T: return newNode(ID_TYPE, (void*) getIntId(token->data));
    case NONE_T: return newNode2(NONE_TYPE, 0);
    default: return dummyNode ? dummyNode : (dummyNode = newNode(__DUMMY_TYPE, 0));
  }
}

#define M(TYPE) match(t, ip, TYPE)
#define M2(T1,T2) M(T1) && M(T2)

Node* match(List* t, int* ip, Token_t type) {
  if (listSize(t) <= *ip) return 0;
  Token* token = (Token*) listGet(t, *ip);
  if (token->type == type) {
    (*ip)++;
    return tokenToNode(token);
  }
  return 0;
}

Node* stmt(List* t, int* ip) {
  Node *n1, *n2, *n3, *n4;
  int i0 = *ip;
  if ((n1 = expr(t, ip)) && M(SEMICOLON_T)) {
    return n1;
  } else if ((*ip = i0), M2(IF_T, OP_B_T) && (n1 = expr(t, ip)) && M(CLO_B_T) && (n2 = block(t, ip))) {
    int i1 = *ip;
    if (M(ELSE_T) && (n3 = block(t, ip))) {
      return newNode2(IF_TYPE, 3, n1, n2, n3);
    } else {
      *ip = i1;
      return newNode2(IF_TYPE, 2, n1, n2);
    }
  } else if ((*ip = i0), M2(FOR_T, OP_B_T) && (n1 = expList(t, ip)) && M(SEMICOLON_T) 
      && (n2 = expList(t, ip)) && M(SEMICOLON_T) && (n3 = expList(t, ip))
      && M(CLO_B_T) && (n4 = block(t, ip))) {
    return newNode2(FOR_TYPE, 4, n1, n2, n3, n4);
  } else if ((*ip = i0), M2(FOR_T, OP_B_T) && (n1 = M(ID_T)) && M(COLON_T) && (n2 = expr(t, ip))
        && M(CLO_B_T) && (n3 = block(t, ip))) {
    return newNode2(FOREACH_TYPE, 3, n1, n2, n3);
  } else if ((*ip = i0), M2(WHILE_T, OP_B_T) && (n1 = expr(t, ip)) && M(CLO_B_T) && (n2 = block(t, ip))) {
    return newNode2(WHILE_TYPE, 2, n1, n2);
  } else if ((*ip = i0), M(FUN_T) && (n1 = M(ID_T)) && M(OP_B_T) && (n2 = idList(t, ip)) && M(CLO_B_T)
      && M(OP_CB_T) && (n3 = stmts(t, ip)) && M(CLO_CB_T)) {
    markTailRecursions(n3);
    return newNode2(FUN_TYPE, 3, n1, n2, n3);
  } else if ((*ip = i0), M(LOCAL_T) && (n1 = idList(t, ip)) && M(SEMICOLON_T)) {
    return newNode2(LOCAL_TYPE, 1, n1);
  } else if ((*ip = i0), M(TRY_T) && (n1 = block(t, ip)) && M(CATCH_T) && M(OP_B_T)
      && (n2 = M(ID_T)) && M(CLO_B_T) && (n3 = block(t, ip))) {
    int i1 = *ip;
    if (M(FINALLY_T) && (n4 = block(t, ip))) {
      return newNode2(TRY_TYPE, 4, n1, n2, n3, n4);
    } else {
      *ip = i1;
      return newNode2(TRY_TYPE, 3, n1, n2, n3);
    }
  } else if ((*ip = i0), M(THROW_T) && (n1 = expr(t, ip)) && M(SEMICOLON_T)) {
    return newNode2(THROW_TYPE, 1, n1);
  } else if ((*ip = i0), M(RETURN_T) && (n1 = expr(t, ip)) && M(SEMICOLON_T)) {
    return newNode2(RETURN_TYPE, 1, n1);
  } else if ((*ip = i0), M(RETURN_T) && M(SEMICOLON_T)) {
    return newNode2(RETURN_TYPE, 1, newNode2(NONE_TYPE, 0));
  } else if ((*ip = i0), M2(CONTINUE_T, SEMICOLON_T)) {
    return newNode2(CONTINUE_TYPE, 0);
  } else if ((*ip = i0), M2(BREAK_T, SEMICOLON_T)) {
    return newNode2(BREAK_TYPE, 0);
  } else if ((*ip = i0), M(IMPORT_T) && (n1 = M(ID_T)) && M(SEMICOLON_T)) {
    return newNode2(IMPORT_TYPE, 1, n1);
  }
  return 0;
}

Node* idList(List* t, int *ip) {
  int i0 = *ip;
  Node* n;
  if (n = nonEmptyIdList(t, ip)) {
    return n;
  } else {
    *ip = i0;
    return newNode2(ID_LIST_TYPE, 0);
  }
}

Node* nonEmptyIdList(List* t, int *ip) {
  int i0 = *ip;
  Node *n1, *n2;
  if ((n1 = M(ID_T)) && M(COMMA_T) && (n2 = nonEmptyIdList(t, ip))) {
    listPushFront(n2->data, n1);
    return n2;
  } else if ((*ip = i0), n1 = M(ID_T)) {
    return newNode2(ID_LIST_TYPE, 1, n1);
  }
  return 0;
}

// module access
Node* moduleAccess(List* t, int* ip) {
  int i0 = *ip;
  Node *n1, *n2;
  if ((n1 = M(ID_T)) && M(DOT_T) && (n2 = moduleAccess(t, ip))) {
    listPushFront(n2->data, n1);
    return n2;
  } else if ((*ip = i0), (n1 = M(ID_T)) && M(DOT_T) && (n2 = M(ID_T))) {
    return newNode2(MODULE_ACCESS_TYPE, 2, n1, n2);
  }
  return 0;
}

Node* listExpr(List* t, int* ip) {
  Node* n;
  if (M(OP_SB_T) && (n = expList(t, ip)) && M(CLO_SB_T)) {
    n->type = LIST_TYPE;
    return n;
  }
  return 0;
}

List* listAccessOrCallInternal(List* t, int* ip) {
  int i0 = *ip;
  Node *n;
  List *l;
  if (M(OP_SB_T) && (n = expr(t, ip)) && M(CLO_SB_T)) {
    n = newNode2(LIST_ACCESS_TYPE, 2, (void*) 0, n);
    l = listAccessOrCallInternal(t, ip);
    listPushFront(l, n);
    return l;
  } else if ((*ip = i0), M(OP_B_T) && (n = expList(t, ip)) && M(CLO_B_T)) {
    n = newNode2(CALL_TYPE, 2, (void*) 0, n);
    l = listAccessOrCallInternal(t, ip);
    listPushFront(l, n);
    return l;
  } else if ((*ip = i0), M(ADDADD_T)) {
    n = newNode2(ADDADD_TYPE, 1, (void*) 0);
    // no [] and () can follow ++
    l = newList();
    listPushFront(l, n);
    return l;
  }
  // Restore *ip value if no matching found.
  // This is different from normal production matcher, because there is no "maching failure".
  *ip = i0; 
  return newList();
}

Node* buildTree(Node* fst, List* rst) {
  int len = listSize(rst), i;
  for (i = 0; i < len; i++) {
    Node* n = listGet(rst, i);
    listSet(n->data, 0, fst);
    fst = n;
  }
  return fst;
}
// id, string, int literal, list literal
Node* expr0(List* t, int* ip) {
  int i0 = *ip;
  Node* n;
  List* l;
  if (n = M(INT_T)) {
    return n;
  } else if ((*ip = i0), (n = M(NONE_T))) {
    return n;
  } else if ((*ip = i0), M(SUB_T) && (n = M(INT_T))) {
    n->data = (void*) (-(long) n->data);
    return n;
  } else if ((*ip = i0), M(OP_B_T) && (n = expr(t, ip)) && M(CLO_B_T)) {
    return buildTree(n, listAccessOrCallInternal(t, ip));
  } else if ((*ip = i0), (n = moduleAccess(t, ip))) {
    return buildTree(n, listAccessOrCallInternal(t, ip));
  } else if ((*ip = i0), (n = M(ID_T))) {
    return buildTree(n, listAccessOrCallInternal(t, ip));
  } else if ((*ip = i0), (n = M(STRING_T))) {
    return buildTree(n, listAccessOrCallInternal(t, ip));
  } else if ((*ip = i0), (n = listExpr(t, ip))) {
    return buildTree(n, listAccessOrCallInternal(t, ip));
  }
  return 0;
}

List* mulDivModInternal(List* t, int* ip) {
  int i0 = *ip;
  List* l;
  Node* n;
  if (M(MUL_T) && (n = expr0(t, ip))) {
    l = mulDivModInternal(t, ip);
    listPushFront(l, newNode2(MUL_TYPE, 2, 0, n));
    return l;
  } else if ((*ip = i0), M(DIV_T) && (n = expr0(t, ip))) {
    l = mulDivModInternal(t, ip);
    listPushFront(l, newNode2(DIV_TYPE, 2, 0, n));
    return l;
  } else if ((*ip = i0), M(MOD_T) && (n = expr0(t, ip))) {
    l = mulDivModInternal(t, ip);
    listPushFront(l, newNode2(MOD_TYPE, 2, 0, n));
    return l;
  } 
  *ip = i0;
  return newList();
}

// * / %
Node* expr1(List* t, int* ip) {
  Node *n;
  if (n = expr0(t, ip)) {
    return buildTree(n, mulDivModInternal(t, ip));
  }
  return 0;
}

List* addSubInternal(List* t, int* ip) {
  int i0 = *ip;
  List* l;
  Node* n;
  if (M(ADD_T) && (n = expr1(t, ip))) {
    l = addSubInternal(t, ip);
    listPushFront(l, newNode2(ADD_TYPE, 2, 0, n));
    return l;
  } else if ((*ip = i0), M(SUB_T) && (n = expr1(t, ip))) {
    l = addSubInternal(t, ip);
    listPushFront(l, newNode2(SUB_TYPE, 2, 0, n));
    return l;
  } 
  *ip = i0;
  return newList();
}
// + -
Node* expr2(List* t, int* ip) {
  Node *n;
  if (n = expr1(t, ip)) {
    return buildTree(n, addSubInternal(t, ip));
  }
  return 0;
}

// = +=
Node* assignmentExpr(List* t, int* ip) {
  Node *n1, *n2;
  if ((n1 = expr0(t, ip))) {
    int i0 = *ip;
    if (M(ASSIGN_T) && (n2 = expr(t, ip))) {
      return newNode2(ASSIGN_TYPE, 2, n1, n2);
    } else if ((*ip = i0), M(ADDEQ_T) && (n2 = expr(t, ip))) {
      return newNode2(ADDEQ_TYPE, 2, n1, n2);
    }
  }
  return 0;
}

Node* expr3(List* t, int* ip) {
  Node *n1, *n2;
  int i0 = *ip;
  if (M(NOT_T) && (n1 = expr3(t, ip))) {
    return newNode2(NOT_TYPE, 1, n1);
  } else if ((*ip = i0), (n1 = expr2(t, ip))) {
    int i1 = *ip;
    if (M(EQ_T) && (n2 = expr2(t, ip))) {
      return newNode2(EQ_TYPE, 2, n1, n2);
    } else if ((*ip = i1), M(NE_T) && (n2 = expr2(t, ip))) {
      return newNode2(NE_TYPE, 2, n1, n2);
    } else if ((*ip = i1), M(GE_T) && (n2 = expr2(t, ip))) {
      return newNode2(GE_TYPE, 2, n1, n2);
    } else if ((*ip = i1), M(LE_T) && (n2 = expr2(t, ip))) {
      return newNode2(LE_TYPE, 2, n1, n2);
    } else if ((*ip = i1), M(GT_T) && (n2 = expr2(t, ip))) {
      return newNode2(GT_TYPE, 2, n1, n2);
    } else if ((*ip = i1), M(LT_T) && (n2 = expr2(t, ip))) {
      return newNode2(LT_TYPE, 2, n1, n2);
    }
    *ip = i1;
    return n1;
  }
}

List* andInternal(List* t, int* ip) {
  int i0 = *ip;
  List* l;
  Node* n;
  if (M(AND_T) && (n = expr3(t, ip))) {
    l = andInternal(t, ip);
    listPushFront(l, newNode2(AND_TYPE, 2, 0, n));
    return l;
  } 
  *ip = i0;
  return newList();
}

Node* expr4(List* t, int *ip) {
  Node *n;
  if (n = expr3(t, ip)) {
    return buildTree(n, andInternal(t, ip));
  }
  return 0;
}

List* orInternal(List* t, int* ip) {
  int i0 = *ip;
  List* l;
  Node* n;
  if (M(OR_T) && (n = expr4(t, ip))) {
    l = orInternal(t, ip);
    listPushFront(l, newNode2(OR_TYPE, 2, 0, n));
    return l;
  } 
  *ip = i0;
  return newList();
}

Node* expr5(List* t, int * ip) {
  Node *n;
  if (n = expr4(t, ip)) {
    return buildTree(n, orInternal(t, ip));
  }
  return 0;
}

Node* lambdaExpr(List* t, int* ip) {
  Node *n1, *n2;
  if (M2(LAMBDA_T, OP_B_T) && (n1 = idList(t, ip)) && M2(CLO_B_T, OP_CB_T) && (n2 = stmts(t, ip)) && M(CLO_CB_T)) {
    return newNode2(FUN_TYPE, 3, newNode(ID_TYPE, (void*) getIntId("lambda")), n1, n2);
  }
  return 0;
}

Node* timeExpr(List* t, int* ip) {
  Node* n;
  if (M2(TIME_T, OP_B_T) && (n = expr(t, ip)) && M(CLO_B_T)) {
    return newNode2(TIME_TYPE, 1, n);
  }
  return 0;
}

Node* expr(List* t, int* ip) {
  int i0 = *ip;
  Node* n;
  if (n = assignmentExpr(t, ip)) {
    return n;
  } else if ((*ip = i0), (n = lambdaExpr(t, ip))) {
    return n;
  } else if ((*ip = i0), (n = timeExpr(t, ip))) {
    return n;
  }
  *ip = i0;
  return expr5(t, ip);
}

Node* block(List* t, int* ip) {
  if (listSize(t) <= *ip) return 0;
  int i0 = *ip;
  Node* n;
  if (M(OP_CB_T) && (n = stmts(t, ip)) && M(CLO_CB_T)) {
    return n;
  } else if ((*ip = i0), (n = stmt(t, ip))) {
    return n;
  }
  return 0;
}

Node* expList(List* t, int *ip) {
  if (listSize(t) < *ip) return 0;
  int i0 = *ip;
  Node* n;
  if (n = nonEmptyExpList(t, ip)) {
    return n;
  } else {
    *ip = i0;
    return newNode2(EXP_LIST_TYPE, 0);
  }
}

Node* nonEmptyExpList(List* t, int *ip) {
  if (listSize(t) <= *ip) return 0;
  int i0 = *ip;
  Node *n1, *n2;
  if ((n1 = expr(t, ip)) && M(COMMA_T) &&(n2 = nonEmptyExpList(t, ip))) {
    listPushFront(n2->data, n1);
    return n2;
  } else if ((*ip = i0), n1 = expr(t, ip)) {
    return newNode2(EXP_LIST_TYPE, 1, n1);
  }
  return 0;
}

#undef M
#undef M2
