#include "parser.h"
#include "util.h"
#include "builtinFun.h"
#include "value.h"

Node* parse(List* tokens) {
  int idx = 0;;
  Node* tree = stmts(tokens, &idx);
  if (idx < listSize(tokens)) {
    int i;
    for (i = 0; i < idx; i++) {
      printf("%s\n", tokenTypeToStr(((Token*) listGet(tokens, i))->type));
    }
    error("failed to parse the while program.");
  }
  return post_process(tree);
}

Node* post_process(Node* tree) {
  // TODO
  return tree;
}

Node* nonEmptyStmts(List* t, int* ip) {
  Node* fst, *rst;
  if ((fst = stmt(t, ip)) && (rst = stmts(t, ip))) {
    listPushFront(rst->data, fst);
    return rst;
  }
  return 0;
}

Node* stmts(List* t, int* ip) {
  int i0 = *ip;
  Node* n;
  if (n = nonEmptyStmts(t, ip)) {
    return n;
  }
  return newNode2(STMTS_TYPE, 0);
}


Node* tokenToNode(Token* token) {
  static Node* dummyNode = 0;
  switch (token->type) {
    case INT_T: return newNode(INT_TYPE, token->data);
    case STRING_T: return newNode(STRING_TYPE, (void*) getIntId(token->data));
    case ID_T: return newNode(ID_TYPE, (void*) getIntId(token->data));
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
Node* expr0_0(List* t, int* ip) {
  int i0 = *ip;
  Node *n1, *n2;
  if ((n1 = M(ID_T)) && M(DOT_T) && (n2 = expr0_0(t, ip))) {
    listPushFront(n2->data, n1);
    return n2;
  } else if ((*ip = i0), (n1 = M(ID_T)) && M(DOT_T) && (n2 = M(ID_T))) {
    return newNode2(MODULE_ACCESS_TYPE, 2, n1, n2);
  }
  return 0;
}

Node* expr0_1(List* t, int* ip) {
  int i0 = *ip;
  Node* n;
  if (n = M(ID_T)) {
    return n;
  } else if ((*ip = i0), (n = M(INT_T))) {
    return n;
  } else if ((*ip = i0), (n = M(STRING_T))) {
    return n;
  }
  return 0;
}

Node* expr0(List* t, int* ip) {
  int i0 = *ip;
  Node* n;
  if (n = expr0_0(t, ip)) {
    return n;
  } else if ((*ip = i0), (n = expr0_1(t, ip))) {
    return n;
  }
  return 0;
}

// * / %
Node* expr1(List* t, int* ip) {
  Node *n1, *n2;
  if (n1 = expr0(t, ip)) {
    int i0 = *ip;
    if (M(MUL_T) && (n2 = expr1(t, ip))) {
      return newNode2(MUL_TYPE, 2, n1, n2);
    } else if ((*ip = i0), M(DIV_T) && (n2 = expr1(t, ip))) {
      return newNode2(DIV_TYPE, 2, n1, n2);
    } else if ((*ip = i0), M(MOD_T) && (n2 = expr1(t, ip))) {
      return newNode2(MOD_TYPE, 2, n1, n2);
    } else {
      *ip = i0;
      return n1;
    }
  }
  return 0;
}

// + -
//
Node* expr2(List* t, int* ip) {
  Node *n1, *n2;
  if (n1 = expr1(t, ip)) {
    int i0 = *ip;
    if (M(ADD_T) && (n2 = expr2(t, ip))) {
      return newNode2(ADD_TYPE, 2, n1, n2);
    } else if ((*ip = i0), M(SUB_T) && (n2 = expr2(t, ip))) {
      return newNode2(SUB_TYPE, 2, n1, n2);
    } else {
      *ip = i0;
      return n1;
    }
  }
  return 0;
}

Node* expr(List* t, int* ip) {
  return expr2(t, ip);
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

void printAst(Node* ast) {
  printf("%s\n", valueToString(nodeToListValue(ast)));
}

#undef M
#undef M2

#ifdef BUILD_PARSER
int main(int argc, char** args) {
  init(argc, args);
  if (argc != 2) {
    error("One argument (input file) required.");
  }
  char* src = args[1];
  char* code = readFile(src);
  List* tokens = tokenize(code);
  Node* tree = parse(tokens);
  if (!tree) {
    error("failed to parse file %s\n", src);
  }
  printAst(tree);
  return 0;
}
#endif
