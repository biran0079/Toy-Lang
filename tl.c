#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
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
    listPush(l, va_arg(v, Node*));
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

Closure* newClosure(Node* f, Env* e){
  Closure* res = MALLOC(Closure);
  res->f = f;
  res->e = e;
  return res;
}

Value* newFunValue(Node* t, Env* e){
  Value* res=MALLOC(Value);
  res->type = FUN_VALUE_TYPE;
  res->data = newClosure(t, e);
  return res;
}

Env* newEnv(Env* parent){
  Env* res = MALLOC(Env);
  res->t = newHashTable();
  res->parent = parent;
  res->loopStates = newList();
  return res;
}

Value* envGet(Env* e, char* key){
  if(!e)return 0;
  Value* res = (Value*) hashTableGet(e->t, key);
  return res ? res : envGet(e->parent, key);
}

void envPut(Env* e, char* key, Value* value){
  Env* e2 = e;
  while(e2){
    if(hashTableGet(e2->t, key)){
      hashTablePut(e2->t, key, (void*) value);
      return;
    }else{
      e2 = e2->parent;
    }
  }
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
      printValue(eval(e, chld(p, 0)));
      printf("\n");
      return 0;
    case LEN_TYPE: {
      Value* v = eval(e, chld(p, 0));
      if(v->type != LIST_VALUE_TYPE) error("len can only apply to list type\n");
      return newIntValue(listSize((List*) v->data));
    }
    case LIST_TYPE: {
      Node* t = chld(p, 0);             
      List* vs = newList();
      for(i=0;i<chldNum(t);i++)
        listPush(vs, eval(e, chld(t,i)));
      return newListValue(vs);
    }
    case LIST_ACCESS_TYPE:{
      List* l = eval(e, chld(p, 0))->data;                      
      Value* idx = eval(e, chld(p, 1));
      if(!idx) error("none value for array index encountered\n");
      return listGet(l, (int) idx->data);
    }
    case LIST_ASSIGN_TYPE:{
      List* l = eval(e, chld(p, 0))->data;                      
      listSet(l, (int) eval(e, chld(p, 1))->data, eval(e, chld(p, 2)));
      return listGet(l, (int) eval(e, chld(p, 1))->data);
    }
    case APP_TYPE: {
      Value* closureValue = envGet(e, chld(p, 0)->data);
      if(!closureValue) error("fun value is none in function application\n");
      Closure *c = closureValue->data;
      Node* f = c->f, *args = chld(p, 1);
      Env* e2 = newEnv(0);
      Node *ids = chld(f,1);
      if(chldNum(ids)!=chldNum(args)) error("function parameter number incorrecr\n");
      for(i=0; i<chldNum(ids); i++)
        envPut(e2, chld(ids, i)->data, eval(e, chld(args, i)));
      e2->parent = c->e;
      Value* ret = (Value*) setjmp(e2->retState);
      if(!ret){
        eval(e2, chld(f, 2));
        return 0;
      }else{
        return ret;
      }
    }
    case RETURN_TYPE:{
         Value* res = 0;
         if(chldNum(p) == 1) {
           res = eval(e, chld(p, 0));
         }
         longjmp(e->retState, (int) res);
     }
    case ID_TYPE:
      return envGet(e, (char*) p->data);
    case INT_TYPE:
      return newIntValue((int) p->data);
    case ASSIGN_TYPE:
      envPut(e, (char*) chld(p, 0)->data, eval(e, chld(p, 1)));
      return envGet(e, (char*) chld(p, 0)->data);
    case ADD_TYPE:
      return valueAdd(eval(e, chld(p, 0)), eval(e, chld(p, 1)));
    case SUB_TYPE:
      return newIntValue(
          (int) eval(e, chld(p, 0))->data -
          (int) eval(e, chld(p, 1))->data);
    case MUL_TYPE:
      return newIntValue(
          (int) eval(e, chld(p, 0))->data *
          (int) eval(e, chld(p, 1))->data);
    case DIV_TYPE: {
      int a = (int) eval(e, chld(p, 0))->data;
      int b = (int) eval(e, chld(p, 1))->data;
      if(!b) error("divisor equals to zero\n");
      return newIntValue(a/b);
    }
    case MOD_TYPE: {
      int a = (int) eval(e, chld(p, 0))->data;
      int b = (int) eval(e, chld(p, 1))->data;
      if(!b) error("divisor equals zero in %%\n");
      return newIntValue(a%b);
    }
    case IF_TYPE:
      if(eval(e, chld(p, 0))->data)
        eval(e, chld(p, 1));
      else if(chldNum(p)==3)
        eval(e, chld(p, 2));
      return 0;
    case WHILE_TYPE:
      {
        jmp_buf buf;
        listPush(e->loopStates, buf);
        int jmp = setjmp(buf);
        while(1){
          if(jmp==0 || jmp==1) {
            // regular case or continue
            int cond = (int) eval(e, chld(p,0))->data;
            if(!cond)break;
            eval(e, chld(p,1));
          } else if(jmp==2) {
            // break
            break;
          } else {
            error("unknown value passed from longjmp\n");
          }
        }
        listPop(e->loopStates);
        return 0;
      }
    case CONTINUE_TYPE:
      longjmp(listLast(e->loopStates), 1);
    case BREAK_TYPE:
      longjmp(listLast(e->loopStates), 2);
    case GT_TYPE:
      return newIntValue(
           (int) eval(e, chld(p, 0))->data > (int) eval(e, chld(p, 1))->data);
    case LT_TYPE:
      return newIntValue(
           (int) eval(e, chld(p, 0))->data < (int) eval(e, chld(p, 1))->data);
    case GE_TYPE:
      return newIntValue(
           (int) eval(e, chld(p, 0))->data >= (int) eval(e, chld(p, 1))->data);
    case LE_TYPE:
      return newIntValue(
           (int) eval(e, chld(p, 0))->data <= (int) eval(e, chld(p, 1))->data);
    case EQ_TYPE:
      return newIntValue(
           valueEquals(eval(e, chld(p, 0)), eval(e, chld(p, 1))));
    case NE_TYPE:
      return newIntValue(
           eval(e, chld(p, 0))->data != eval(e, chld(p, 1))->data);
    case AND_TYPE:
      return newIntValue(
           eval(e, chld(p, 0))->data && eval(e, chld(p, 1))->data);
    case OR_TYPE:
      return newIntValue(
           eval(e, chld(p, 0))->data || eval(e, chld(p, 1))->data);
    case NOT_TYPE:
      return newIntValue(! eval(e, chld(p, 0))->data);
    case FUN_TYPE:
      envPut(e, (char*) chld(p, 0)->data, newFunValue(p, e));
      return envGet(e, (char*) chld(p, 0)->data);
    default:
      error("cannot eval unknown node type\n");
  }
}

Value* newListValue(List* list) {
  Value* res = MALLOC(Value);
  res->type = LIST_VALUE_TYPE;
  res->data = list;
  return res;
}

int valueEquals(Value* v1, Value* v2){
  if(v1 == v2) return 1;
  if(!v1 || !v2) return 0;
  if(v1->type != v2->type) return 0;
  switch(v1->type){
    case INT_VALUE_TYPE:
    case FUN_VALUE_TYPE: return v1->data == v2->data;
    case LIST_VALUE_TYPE: {
      List* l1 = v1->data;
      List* l2 = v2->data;
      int len = listSize(l1);
      int i;
      if(len!=listSize(l2))return 0;
      for(i=0;i<len;i++)
        if(!valueEquals(listGet(l1, i), listGet(l2, i)))
          return 0;
      return 1;
    }
    default: error("unknown value type passed to valueEquals\n");
  }
}

Value* valueAdd(Value* v1, Value* v2) {
  switch(v1->type){
    case INT_VALUE_TYPE:
      switch(v2->type){
        case INT_VALUE_TYPE: 
          return newIntValue((int) v1->data + (int) v2->data);
        default: error("int can only add to int\n");
      }
    case LIST_VALUE_TYPE:{
      List* res = listCopy(v1->data);
      int i;
      switch(v2->type){
        case LIST_VALUE_TYPE: {
          List* l = v2->data;
          for(i=0;i<listSize(l);i++)
            listPush(res, listGet(l, i));
          break;
        }
        default:
          listPush(res, v2);
          break;
      }
      return newListValue(res);
    }
    case FUN_VALUE_TYPE: error("Function value cannot add to anything\n");
    default: error("unknown value type passed to valueAdd\n");
  }
}

void printValue(Value* v) {
  if(!v){
    printf("none");
    return;
  }
  Node* t;
  int i;
  switch(v->type) {
    case INT_TYPE:
      printf("%d", v->data);
      break;
    case FUN_VALUE_TYPE: {
      Node* f = ((Closure*) v->data)->f;
      printf("fun %s(", chld(f, 0)->data);
      t = chld(f, 1);
      for(i=0;i<chldNum(t);i++){
        if(i)printf(" ");
        printf("%s", chld(t, i)->data);
      }
      printf(")");
      break;
    }
    case LIST_VALUE_TYPE: {
      List* l = v->data;
      printf("[");
      for(i=0; i<listSize(l);i++){
        if(i)printf(", ");
        printValue(listGet(l, i));
      }
      printf("]");
      break;
    }
    default: error("cannot print unknown value type\n");
  }
}

void error(char* msg) {
  fprintf(stderr, msg);
  exit(-1);
}
