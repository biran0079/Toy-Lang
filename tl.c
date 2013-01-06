#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <time.h>
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

long strToLong(char* s){
  long res = 0;
  int negative = 0;
  if(*s == '-') {
    negative = 1;
    s++;
  }
  while(*s) {
    res*=10;
    res+=*s-'0';
    s++;
  }
  return negative ? -res : res;
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

Value* newIntValue(long x){
  Value* res=MALLOC(Value);
  res->type = INT_VALUE_TYPE;
  res->data = (void*) x;
  return res;
}

Value* newStringValue(char *s){
  Value* res=MALLOC(Value);
  res->type = STRING_VALUE_TYPE;
  res->data = (void*) s;
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
  res->returnValue = 0;
  res->tailCall = 0;
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

void clearEnv(Env* e){
  hashTableClear(e->t);
  listClear(e->loopStates);
  e->parent=0;
  e->returnValue = 0;
  e->tailCall = 0;
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
    case EXP_LIST_TYPE: {
      Value* res = 0;
      for(i=0;i<chldNum(p);i++)
        res = eval(e, chld(p, i));
      return res;
    }
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
      if(!idx) error("list index cannot be none\n");
      return listGet(l, (long) idx->data);
    }
    case LIST_ASSIGN_TYPE: {
      List* l = eval(e, chld(p, 0))->data;                      
      Value* idx = eval(e, chld(p, 1));
      if(!idx) error("list index cannot be none\n");
      listSet(l, (long) idx->data, eval(e, chld(p, 2)));
      return listGet(l, (long) eval(e, chld(p, 1))->data);
    }
    case LIST_ADDEQ_TYPE: {
      List* l = eval(e, chld(p, 0))->data;                      
      Value* idx = eval(e, chld(p, 1));
      if(!idx) error("list index cannot be none\n");
      Value* e1 = listGet(l, (long) idx->data);
      Value* e2 = eval(e, chld(p, 2));
      if(!e1 || !e2) error("+= does not work for none\n");
      Value* res = 0;
      switch(e1->type) {
        case INT_VALUE_TYPE:
          res = valueAdd(e1, e2);
          listSet(l, (long) idx->data, res);
          break;
        case LIST_VALUE_TYPE:
          if(e2->type == LIST_VALUE_TYPE) {
            List* l = (List*) e2->data;
            for(i=0;i<listSize(l);i++)
              listPush((List*) e1->data, listGet(l, i));
          } else {
            listPush((List*) e1->data, e2);
          }
          res=e1;
          break;
        default: error("unknown type for operator +=");
      }
      return res;
    }
    case TAIL_CALL_TYPE: {
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
      e->tailCall = newClosure(f, e2);
      longjmp(e->retState, 2);
    }
    case CALL_TYPE: {
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
      while(1){
        int ret = setjmp(e2->retState);
        if(!ret){
          eval(e2, chld(f, 2));
          // if reach here, no return statement is called, so none is returned 
          return 0;
        } else if(ret == 1) {
          return e2->returnValue;
        } if(ret == 2) {
          // tail recursive call
          Closure* tc = e2->tailCall;
          f = tc->f;
          e2 = tc->e;
          continue;
        } else {
          error("unknown  return value from longjmp in function call\n");
        }
      }
    }
    case RETURN_TYPE:{
         Value* res = 0;
         if(chldNum(p) == 1) {
           res = eval(e, chld(p, 0));
         }
         e->returnValue = res;
         longjmp(e->retState, 1);
     }
    case ID_TYPE:
      return envGet(e, (char*) p->data);
    case INT_TYPE:
      return newIntValue((long) p->data);
    case STRING_TYPE:
      return newStringValue((char*) p->data);
    case ASSIGN_TYPE: {
      Value* res = eval(e, chld(p, 1));
      envPut(e, (char*) chld(p, 0)->data, res);
      return res;
    }
    case ADDEQ_TYPE: {
      char* key = (char *) chld(p, 0)->data;
      Value* e1 = envGet(e, key);
      Value* e2 = eval(e, chld(p, 1));
      if(!e1 || !e2) error("+= does not work for none\n");
      Value* res = 0;
      switch(e1->type) {
        case INT_VALUE_TYPE:
          res = valueAdd(e1, e2);
          envPut(e, key, res);
          break;
        case LIST_VALUE_TYPE:
          if(e2->type == LIST_VALUE_TYPE) {
            List* l = (List*) e2->data;
            for(i=0;i<listSize(l);i++)
              listPush((List*) e1->data, listGet(l, i));
          }else{
            listPush((List*) e1->data, e2);
          }
          res=e1;
          break;
        default: error("unknown type for operator +=");
      }
      return res;
    }
    case ADD_TYPE:
      return valueAdd(eval(e, chld(p, 0)), eval(e, chld(p, 1)));
    case SUB_TYPE:
      return valueSub(eval(e, chld(p, 0)), eval(e, chld(p, 1)));
    case MUL_TYPE:
      return valueMul(eval(e, chld(p, 0)), eval(e, chld(p, 1)));
    case DIV_TYPE: 
      return valueDiv(eval(e, chld(p, 0)), eval(e, chld(p, 1)));
    case MOD_TYPE: 
      return valueMod(eval(e, chld(p, 0)), eval(e, chld(p, 1)));
    case IF_TYPE:
      if(eval(e, chld(p, 0))->data)
        eval(e, chld(p, 1));
      else if(chldNum(p)==3)
        eval(e, chld(p, 2));
      return 0;
    case FOR_TYPE:
      {
        jmp_buf buf;
        listPush(e->loopStates, buf);
        eval(e, chld(p,0));
        int jmp = setjmp(buf);
        while(1){
          if(jmp==0){
            // regular case
            int cond = (long) eval(e, chld(p, 1))->data;
            if(!cond) break;
            eval(e, chld(p, 3));
            eval(e, chld(p, 2));
          } else if(jmp==1) {
            // continue
            eval(e, chld(p, 2));
            jmp=0;
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
    case WHILE_TYPE:
      {
        jmp_buf buf;
        listPush(e->loopStates, buf);
        int jmp = setjmp(buf);
        while(1){
          if(jmp==0 || jmp==1) {
            // regular case or continue
            int cond = (long) eval(e, chld(p, 0))->data;
            if(!cond) break;
            eval(e, chld(p, 1));
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
           (long) eval(e, chld(p, 0))->data > (long) eval(e, chld(p, 1))->data);
    case LT_TYPE:
      return newIntValue(
           (long) eval(e, chld(p, 0))->data < (long) eval(e, chld(p, 1))->data);
    case GE_TYPE:
      return newIntValue(
           (long) eval(e, chld(p, 0))->data >= (long) eval(e, chld(p, 1))->data);
    case LE_TYPE:
      return newIntValue(
           (long) eval(e, chld(p, 0))->data <= (long) eval(e, chld(p, 1))->data);
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
    case TIME_TYPE: {
        clock_t st = clock();                
        Value* res = eval(e, chld(p, 0));
        fprintf(stderr, "time: %lf secs\n", (clock() - st) * 1.0 / CLOCKS_PER_SEC);
        return res;
    }
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

Value* valueSub(Value* v1, Value* v2) {
  if(!v1 || !v2) error("- operator does not work for none\n");
  if(v1->type != INT_VALUE_TYPE || v2->type != INT_VALUE_TYPE)
    error("- operator only works for int");
  return newIntValue((long) v1->data - (long) v2->data);
}

Value* valueMul(Value* v1, Value* v2) {
  if(!v1 || !v2) error("* operator does not work for none\n");
  if(v1->type != INT_VALUE_TYPE || v2->type != INT_VALUE_TYPE)
    error("* operator only works for int");
  return newIntValue((long) v1->data * (long) v2->data);
}

Value* valueDiv(Value* v1, Value* v2) {
  if(!v1 || !v2) error("/ operator does not work for none\n");
  if(v1->type != INT_VALUE_TYPE || v2->type != INT_VALUE_TYPE)
    error("/ operator only works for int");
  if(!v2->data) error("denominator cannot be zero\n");
  return newIntValue((long) v1->data / (long) v2->data);
}

Value* valueMod(Value* v1, Value* v2) {
  if(!v1 || !v2) error("%% operator does not work for none\n");
  if(v1->type != INT_VALUE_TYPE || v2->type != INT_VALUE_TYPE)
    error("%% operator only works for int");
  if(!v2->data) error("denominator cannot be zero\n");
  return newIntValue((long) v1->data % (long) v2->data);
}


Value* valueAdd(Value* v1, Value* v2) {
  if(!v1 || !v2) error("+ operator does not work for none\n");
  switch(v1->type){
    case INT_VALUE_TYPE:
      switch(v2->type){
        case INT_VALUE_TYPE: 
          return newIntValue((long) v1->data + (long) v2->data);
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
    case STRING_VALUE_TYPE: {
      printf("%s", v->data);
      break;
    }
    default: error("cannot print unknown value type\n");
  }
}

void error(char* msg) {
  fprintf(stderr, msg);
  exit(-1);
}
