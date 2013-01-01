#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include"tl.h"
#include"list.h"

static Node* createNode() {
  Node* res = MALLOC(Node);
  res->data = (void*) 0;
  return res;
}

void* copy(void* t, int size) {
  void* res = malloc(size);
  memcpy(res, t, size);
  return res;
}

int chldNum(Node* t) {
  return listSize((List*) t->data);
}

Node* chld(Node* e, int i) {
  return listGet((List*) e->data, i);
}

Node* newNode(NodeType t, void* i) {
  Node* res = createNode();
  res->type = t;
  res->data = i;
  return res;
}
Node* newNode2(NodeType t, int n, ... ) {
  Node* res = createNode();
  List* l = newList();
  va_list v;
  int i;
  va_start(v, n);
  for(i=0; i<n; i++) {
    listAdd(l, va_arg(v, Node*));
  }
  res->type = t;
  res->data = (void*) l;
  va_end(v);
  return res;
}

Value* newIntValue(int x){
  Value* res=MALLOC(Value);
  res->type = INT_VALUE_TYPE;
  res->data = (void*) x;
  return res;
}

Env* newEnv(){
  Env* res = MALLOC(Env);
  res->t = newHashTable();
  return res;
}

Value* envGet(Env* e, char* key){
  return (Value*) hashTableGet(e->t, key);
}

void envPut(Env* e, char* key, Value* value){
  hashTablePut(e->t, key, (void*) value);
}

Value* eval(Env* e, Node* p) {
  List* l;
  Value* res = 0;
  int i;
  switch(p->type){
    case STMTS_TYPE:
      l = (List*) p->data;
      for(i=0;i<listSize(l);i++){
        eval(e, listGet(l,i));
      }
      return 0;
    case PRINT_TYPE:
      printValue(eval(e, (Node*) p->data));
      return 0;
    case ID_TYPE:
      return envGet(e, (char*) p->data);
    case INT_TYPE:
      return newIntValue((int) p->data);
    case ASSIGN_TYPE:
      envPut(e, (char*) chld(p, 0)->data, eval(e, chld(p, 1)));
      return envGet(e, (char*) chld(p, 0)->data);
    case ADD_TYPE:
      return newIntValue(
          (int) eval(e, chld(p, 0))->data + 
          (int) eval(e, chld(p, 1))->data);
    case SUB_TYPE:
      return newIntValue(
          (int) eval(e, chld(p, 0))->data -
          (int) eval(e, chld(p, 1))->data);
    case MUL_TYPE:
      return newIntValue(
          (int) eval(e, chld(p, 0))->data *
          (int) eval(e, chld(p, 1))->data);
    case DIV_TYPE:
      return newIntValue(
          (int) eval(e, chld(p, 0))->data /
          (int) eval(e, chld(p, 1))->data);
    default:
      fprintf(stderr, "cannot eval unknown node type\n");
      exit(-1);
  }
}

void printValue(Value* v) {
  if(!v){
    printf("none\n");
    return;
  }
  switch(v->type) {
    case INT_TYPE:
      printf("%d\n", v->data);
      break;
    default:
      fprintf(stderr, "cannot print unknown value type\n");
      break;
  }
}
