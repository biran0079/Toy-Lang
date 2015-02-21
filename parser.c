#include "parser.h"
#include "util.h"
#include "builtinFun.h"
#include "value.h"

Node* parse(List* tokens) {
  int idx = 0;;
  return post_process(stmts(tokens, &idx));
}

Node* post_process(Node* tree) {
  // TODO
  return tree;
}

Node* stmts(List* t, int* ip) {
  if (listSize(t) == *ip) {
    return newNode2(STMTS_TYPE, 0);
  } else {
    Node* fst = stmt(t, ip);
    fst || error("failed to parse stmt\n");
    Node* rst = stmts(t, ip);
    rst || error("failed to parse stmts\n");
    listPushFront(rst->data, fst);
    return rst;
  }
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
  Token* token = (Token*) listGet(t, *ip);
  if (token->type == type) {
    (*ip)++;
    return tokenToNode(token);
  }
  return 0;
}

Node* stmt(List* t, int* ip) {
  Node *n1, *n2, *n3;
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
  }
  return 0;
}

// module access
Node* expr0(List* t, int* ip) {
  int i0 = *ip;
  Node *n1, *n2;
  if ((n1 = M(ID_T)) && M(DOT_T) && (n2 = expr0(t, ip))) {
    listPushFront(n2->data, n1->data);
    return n2;
  } else if ((*ip = i0), (n1 = M(ID_T)) && M(DOT_T) && (n2 = M(ID_T))) {
    return newNode2(MODULE_ACCESS_TYPE, 2, n1, n2);
  }
  return 0;
}

Node* expr(List* t, int* ip) {
  return expr0(t, ip);
}

Node* block(List* t, int* ip) {
  int i0 = *ip;
  Node* n;
  if (M(OP_CB_T) && (n = stmts(t, ip)) && M(CLO_CB_T)) {
    return n;
  } else if ((*ip = i0), (n = stmt(t, ip))) {
    return n;
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
