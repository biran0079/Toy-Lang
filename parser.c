#include "parser.h"
#include "util.h"

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

#define M(TYPE) match(t, ip, TYPE)
#define M2(T1,T2) M(T1) && M(T2)

Token* match(List* t, int* ip, Token_t type) {
  Token* token = (Token*) listGet(t, *ip);
  if (token->type == type) {
    (*ip)++;
    return token;
  }
  return 0;
}

Node* stmt(List* t, int* ip) {
  Node *n1, *n2, *n3;
  int i0 = *ip;
  if ((n1 = expr(t, ip)) && M(SEMICOLON_T)) {
    return n1;
  } else if ((*ip = i0) && M2(IF_T, OP_B_T) && (n1 = expr(t, ip)) && M(CLO_B_T) && (n2 = block(t, ip))) {
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
  Token *t1, *t2;
  Node* n;
  if ((t1 = M(ID_T)) && M(DOT_T) && (n = expr0(t, ip))) {
    listPushFront(n->data, t1->data);
    return n;
  } else if ((*ip = i0) && (t1 = M(ID_T)) && M(DOT_T) && (t2 = M(ID_T))) {
    return newNode2(MODULE_ACCESS_TYPE, 2, t1->data, t2->data);
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
  } else if ((*ip = i0) && (n = stmt(t, ip))) {
    return n;
  }
  return 0;
}

#undef M
#undef M2

#ifdef BUILD_PARSER
int main(int argc, char** args) {
  char* src = args[1];
  char* code = readFile(src);
  List* tokens = tokenize(code);
  Node* tree = parse(tokens);
  if (!tree) {
    error("failed to parse file %s\n", src);
  }
  char* s = catStr(src, ".dot");
  FILE* f=fopen(s, "w");
  if (!f) error("failed to open %s\n", s);
  nodeToDot(f, tree);
  fclose(f);
  free(s);
  return 0;
}
#endif
