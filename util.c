#include "tl.h"

void* copy(void* t, int size) {
  void* res = malloc(size);
  memcpy(res, t, size);
  return res;
}

long strToLong(char* s){
  long res = 0;
  while(*s) {
    res*=10;
    res+=*s-'0';
    s++;
  }
  return res;
}

char* literalStringToString(char *s) {
  char *ss = malloc(strlen(s)+1);
  int l = 0;
  s++;  // skil "
  while(*s!='"') {
    if(*s == '\\') {
      s++;
      switch(*s){
        case 'n' : ss[l++] = '\n'; break;
        case 'r' : ss[l++] = '\r'; break;
        case '"' : ss[l++] = '"'; break;
        case '\\' : ss[l++] = '\\'; break;
        default:  ss[l++] = *s; break;
      }
    }else{
      ss[l++] = *s;
    }
    s++;
  }
  ss[l++] = 0;
  return realloc(ss, l);
}

int chldNum(Node* t) {
  return listSize((List*) t->data);
}

Node* chld(Node* e, int i) {
  return listGet((List*) e->data, i);
}

extern int newNodeC, newNode2C;

Node* newNode(NodeType t, void* i) {
  newNodeC++;
  Node* res = MALLOC(Node);
  res->type = t;
  res->data = i;
  return res;
}

Node* newNode2(NodeType t, int n, ... ) {
  newNode2C++;
  Node* res = MALLOC(Node);
  List* l = newList();
  va_list v;
  int i;
  va_start(v, n);
  for(i=0; i<n; i++) {
    listPush(l, va_arg(v, Node*));
  }
  res->type = t;
  res->data = (void*) l;
  va_end(v);
  return res;
}

void error(char* format,...) {
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
  exit(-1);
}

void markTailRecursions(Node* t) {
  switch(t->type) {
    case STMTS_TYPE: {
      int n = chldNum(t);
      if(n) markTailRecursions(chld(t, n-1));
      break;
    }
    case IF_TYPE: {
      markTailRecursions(chld(t, 1));
      if(chldNum(t) > 2) // has "else"
        markTailRecursions(chld(t, 2));
      break;
    }
    case RETURN_TYPE: {
      if(chldNum(t)) markTailRecursions(chld(t, 0));
      break;
    }
    case CALL_TYPE: {
      t->type = TAIL_CALL_TYPE;
      break;
    }
    default: break;
  }
}
