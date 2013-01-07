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

char* nodeTypeToString(NodeType type){
  switch(type) {
    case ADD_TYPE: return "+";
    case SUB_TYPE: return "-";
    case MUL_TYPE: return "*";
    case DIV_TYPE: return "/";
    case MOD_TYPE: return "%";
    case GT_TYPE:  return ">";
    case LT_TYPE:  return "<";
    case GE_TYPE:  return ">=";
    case LE_TYPE:  return "<=";
    case EQ_TYPE:  return "==";
    case NE_TYPE:  return "!=";
    case AND_TYPE: return "&&";
    case OR_TYPE:  return "||";
    case LIST_ASSIGN_TYPE:
    case ASSIGN_TYPE: return "=";
    case LIST_ADDEQ_TYPE:
    case ADDEQ_TYPE:  return "+=";
    case TIME_TYPE: return "time";
    case STR_TYPE: return "str";
    case ORD_TYPE: return "ord";
    case LEN_TYPE: return "len";
    case RETURN_TYPE: return "return";
    case EXP_LIST_TYPE: return "exps";
    case ID_LIST_TYPE: return "ids";
    case STMTS_TYPE: return "stmts";
    case LIST_TYPE: return "list";
    case INT_TYPE: return "int";
    case ID_TYPE: return "id";
    case PRINT_TYPE: return "print";
    case IF_TYPE: return "if";
    case NOT_TYPE: return "not";
    case WHILE_TYPE: return "while";
    case ARGS_TYPE: return "args";
    case FUN_TYPE: return "fun";
    case CALL_TYPE: return "call";
    case TAIL_CALL_TYPE: return "tail_call";
    case BREAK_TYPE: return "break";
    case CONTINUE_TYPE: return "continue";
    case LIST_ACCESS_TYPE: return "list_access";
    case FOR_TYPE: return "for";
    case STRING_TYPE: return "string";
    case NONE_TYPE: return "none";
    case FOREACH_TYPE: return "foreach";
    case TRY_TYPE: return "try";
    case THROW_TYPE: return "throw";
    case ADDADD_TYPE: return "(id)++";
    default: error("unknown node type %d\n", type);
  }
}

static char* maybeTruncateString(char *s){
  int len = strlen(s);
  if(len>10){
    char *res = (char*) malloc(14);
    memcpy(res, s, 10);
    res[10] = 0;
    strcat(res, "...");
    return res;
  }
  return s;
}

static int dotNeedEscape(char c){
  return c=='<' || c=='>' || c=='"' || c=='\n' || c=='|';
}

static char* escapeForDot(char*s){
  int i;
  int ct = 0;
  for(i=0;s[i];i++) 
    if(dotNeedEscape(s[i]))
      ct++;
  char *res = (char*) malloc(strlen(s) + ct + 1);
  int j=0;
  for(i=0;s[i];i++){
    if(dotNeedEscape(s[i]))
      res[j++]='\\';
    res[j++]=s[i];
  }
  res[j]=0;
  return res;
}

static int nodeToDotInternal(FILE* o, Node* t, int* nextId);

static int nodeToDotHelper(FILE* o, Node *t, int* nextId, long n, ...){
  int id = *nextId - 1;
  va_list l;
  char* s;
  int i;
  va_start(l, n);
  fprintf(o, "%d [label=\"{%s|{", id, escapeForDot(nodeTypeToString(t->type)));
  for(i=0;i<n;i++){
    if(i) fprintf(o, "|");
    fprintf(o, "<%d> %s", i, va_arg(l, char*));
  }
  fprintf(o, "}}\"];\n");
  for(i=0;i<n;i++) {
    int to = nodeToDotInternal(o, chld(t, i), nextId);
    fprintf(o, "%d:%d -> %d;\n", id, i, to);
  }
  va_end(l);
}

static int nodeToDotInternal(FILE* o, Node* t, int* nextId){
  //printf("%s\n", nodeTypeToString(t->type));
  int id = (*nextId)++;
  switch(t->type) {
    case INT_TYPE: fprintf(o, "%d [label=\"INT(%ld)\"];\n", id, (long) t->data);break;
    case ID_TYPE:  fprintf(o, "%d [label=\"ID(%s)\"];\n", id, (char*) t->data);break;
    case STRING_TYPE: fprintf(o, "%d [label=\"STRING(%s)\"];\n", id, 
                          escapeForDot(maybeTruncateString((char*) t->data)));
                      break;
    case NONE_TYPE: fprintf(o, "%d [label=\"none\"];\n", id);break;
    case BREAK_TYPE: fprintf(o, "%d [label=\"break\"]", id);break;
    case CONTINUE_TYPE: fprintf(o, "%d [label=\"continue\"]", id);break;

    case EXP_LIST_TYPE: 
    case ID_LIST_TYPE:
    case LIST_TYPE:
    case STMTS_TYPE: {
      fprintf(o, "%d [label=\"{%s|{",id, nodeTypeToString(t->type));
      int n = chldNum(t);
      int i;
      for(i=0;i<n;i++){
        if(i) fprintf(o, "|");
        fprintf(o, "<%d> %d", i, i);
      }
      fprintf(o, "}}\"];\n");
      for(i=0;i<n;i++) {
        int to = nodeToDotInternal(o, chld(t, i), nextId);
        fprintf(o, "%d:%d -> %d;\n", id, i, to);
      }
      break;
    }
    case ASSIGN_TYPE: 
    case ADD_TYPE: 
    case SUB_TYPE:
    case MUL_TYPE:
    case DIV_TYPE:
    case MOD_TYPE:
    case GT_TYPE:
    case LT_TYPE:
    case GE_TYPE:
    case LE_TYPE:
    case EQ_TYPE:
    case NE_TYPE:
    case AND_TYPE:
    case OR_TYPE:
    case ADDEQ_TYPE:
    case LIST_ASSIGN_TYPE:
    case LIST_ADDEQ_TYPE:  nodeToDotHelper(o, t, nextId, 2, "op1", "op2");break;
    case TIME_TYPE:
    case STR_TYPE:
    case ORD_TYPE:
    case LEN_TYPE:
    case THROW_TYPE: 
    case RETURN_TYPE: nodeToDotHelper(o, t, nextId, 1, "exp"); break;
    case NOT_TYPE: nodeToDotHelper(o, t, nextId, 1, "exp"); break;
    case ADDADD_TYPE: nodeToDotHelper(o, t, nextId, 1, "id"); break;
    case PRINT_TYPE: nodeToDotHelper(o, t, nextId, 1, "exps"); break;
    case IF_TYPE: {
      int n = chldNum(t), to;
      if(n==2) {
        nodeToDotHelper(o, t, nextId, 2, "cond", "then");
      }else{
        nodeToDotHelper(o, t, nextId, 3, "cond", "then", "else");
      }
      break;
    }
    case TRY_TYPE: {
      int n = chldNum(t), to;
      if(n==3) {
        nodeToDotHelper(o, t, nextId, 3, "try","excep id", "catch");
      }else{
        nodeToDotHelper(o, t, nextId, 4, "try", "excep id", "catch", "finally");
      }
      break;
    }
    case WHILE_TYPE: nodeToDotHelper(o, t, nextId, 2, "cond", "do"); break;
    case FUN_TYPE: nodeToDotHelper(o, t, nextId, 3, "name", "args", "body"); break; 
    case CALL_TYPE: 
    case TAIL_CALL_TYPE: nodeToDotHelper(o, t, nextId, 2, "name", "args"); break; 
    case LIST_ACCESS_TYPE: nodeToDotHelper(o, t, nextId, 2, "list", "index"); break; 
    case FOR_TYPE: nodeToDotHelper(o, t, nextId, 4, "init", "cond", "incr", "body"); break; 
    case FOREACH_TYPE: nodeToDotHelper(o, t, nextId, 3, "id", "list", "body"); break;
    default: error("unknown type %d in valueToStringInternal\n", t->type);
  }
  return id;
}

void nodeToDot(FILE* o, Node* t) {
  static int id = 0;
  fprintf(o, "digraph G {\nnode [shape = record];\n");
  nodeToDotInternal(o, t, &id);
  fprintf(o, "}\n");
  id = 0;
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
  res->exceptionStates = newList();
  res->exceptionValue = 0;
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

void throwValue(Env* e, Value* v) {
  e->exceptionValue = v;  // pass exception value through enviconment
  if(listSize(e->exceptionStates)) {
    // currently in try block
    longjmp(listLast(e->exceptionStates), 1);
  } else {
    // If has parent stack frame, pass to it. Report error otherwise
    if (e->parent) {
      longjmp(e->retState, 3);
    } else {
      error("uncaught exception:\n%s\n", valueToString(v));
    }
  }
  error("should never reach here\n");
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
    case PRINT_TYPE: {
      Node* exps = chld(p,0);
      int i;
      for(i=0;i<chldNum(exps);i++) {
        if(i)putchar(' ');
        printf("%s", valueToString(eval(e, chld(exps, i))));
      }
      printf("\n");
      return 0;
    }
    case LEN_TYPE: {
      Value* v = eval(e, chld(p, 0));
      switch(v->type) {
        case LIST_VALUE_TYPE: return newIntValue(listSize((List*) v->data));
        case STRING_VALUE_TYPE: return newIntValue(strlen((char*) v->data));
        default: error("unsupported type for 'len' operator\n");
      }
    }
    case STR_TYPE: {
      return newStringValue(valueToString(eval(e, chld(p,0))));
    }
    case ORD_TYPE: {
      Value* v = eval(e, chld(p, 0));
      if(v->type != STRING_VALUE_TYPE || strlen((char*) v->data) != 1)
        error("ord() can only apply to string with length 1\n");
      return newIntValue(*((char*) v->data));
    }
    case NONE_TYPE: return 0;
    case LIST_TYPE: {
      List* vs = newList();
      for(i=0;i<chldNum(p);i++)
        listPush(vs, eval(e, chld(p,i)));
      return newListValue(vs);
    }
    case LIST_ACCESS_TYPE:{
      Value* v = eval(e, chld(p, 0));
      Value* idxValue = eval(e, chld(p, 1));
      if(!idxValue) error("list index cannot be none\n");
      long idx = (long) idxValue->data;
      switch(v->type) {
        case LIST_VALUE_TYPE: {
          List* l = (List*) v->data;                      
          return listGet(l, idx);
        }
        case STRING_VALUE_TYPE: {
          char *s = (char*) v->data;
          char *ss = (char*) malloc(2 * sizeof(char));
          ss[0] = s[idx];
          ss[1] = 0;
          return newStringValue(ss);
        }
      }
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
        case STRING_VALUE_TYPE:
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
      // actuall call is handled by CALL_TYPE
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
        } if(ret == 3) {
          // exception passed out
          Value* v = e2->exceptionValue;
          e2->exceptionValue = 0;
          throwValue(e, v);
        } else {
          error("unknown  return value from longjmp in function call\n");
        }
      }
    }
    case RETURN_TYPE:{
         Value* res = 0;
         res = eval(e, chld(p, 0));
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
        case STRING_VALUE_TYPE:
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
        default: error("unknown type for operator +=\n");
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
    case FOR_TYPE: {
      jmp_buf buf;
      listPush(e->loopStates, buf);
      for(eval(e, chld(p,0)); eval(e, chld(p, 1))->data; eval(e, chld(p, 2))){
          int jmp = setjmp(buf);
          if(jmp==0){
            // regular case
            eval(e, chld(p, 3));
          } else if(jmp==1) {
            // continue
            continue;
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
    case FOREACH_TYPE:{
      jmp_buf buf;
      listPush(e->loopStates, buf);
      Node* id = chld(p, 0);
      Value* lv = eval(e, chld(p, 1));
      if(id->type != ID_TYPE || lv->type != LIST_VALUE_TYPE) {
        error("param type incorrect for for( : ) statement\n ");
      }
      List* l = (List*) lv->data;
      int i, len = listSize(l);
      for(i=0;i<len;i++) {
        envPut(e, (char*) id->data, listGet(l, i));
        int jmp = setjmp(buf);
        if(jmp==0) {
          eval(e, chld(p, 2));
        } else if(jmp==1) {
          continue;
        } else if(jmp==2) {
          break;
        } else {
          error("unknown longjmp state\n");
        }
      }
      listPop(e->loopStates);
      return 0;
    }
    case WHILE_TYPE:
      {
        jmp_buf buf;
        listPush(e->loopStates, buf);
        while(eval(e, chld(p, 0))->data){
          int jmp = setjmp(buf);
          if(jmp==0) {
            // regular case 
            eval(e, chld(p, 1));
          } else if(jmp==1) {
            // continue
            continue;
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
           !valueEquals(eval(e, chld(p, 0)), eval(e, chld(p, 1))));
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
    case TRY_TYPE: {
      jmp_buf buf;
      listPush(e->exceptionStates, buf);
      Node* tryBlock = chld(p, 0);
      char* catchId = (char*) chld(p, 1)->data;
      Node* catchBlock = chld(p, 2);
      Node* finallyBlock = chldNum(p) == 4 ? chld(p, 3) : 0;
      switch(setjmp(buf)){
        case 0: {
          // try block
          eval(e, tryBlock);
          listPop(e->exceptionStates); // pop when out of try block
          break;
        }
        case 1: {
          // catch block
          listPop(e->exceptionStates); // pop when out of try block
          // exception caught        
          Value* v = e->exceptionValue;
          e->exceptionValue = 0;
          envPut(e, catchId, v);
          eval(e, catchBlock);
          break;
        }
        default: error("unknown state passed from longjmp in try statement\n");
                 break;
      }
      if(finallyBlock) eval(e, finallyBlock);
      return 0;
    }
    case THROW_TYPE: {
      Value* v = eval(e, chld(p, 0));           
      throwValue(e, v);
    }
    case ADDADD_TYPE: {
      // i++ type                  
      char* id = (char*) chld(p, 0)->data;
      Value* i = envGet(e, id);
      if(!id) { error("%s is not defined in current environment\n", id); }
      envPut(e, id, newIntValue((long) i->data + 1));
      return i;
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
    case STRING_VALUE_TYPE: 
      return strcmp((char*) v1->data, (char*) v2->data) == 0;
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
    case STRING_VALUE_TYPE: {
      if(v2->type == STRING_VALUE_TYPE) {
        char *s1 = (char*) v1->data, *s2 = (char*) v2->data;
        char *res = (char*) malloc(strlen(s1) + strlen(s2) + 1);
        *res=0;
        strcat(res, s1);
        strcat(res, s2);
        return newStringValue(res);
      } else {
        error("can only add string to string\n");
      }
    }
    case FUN_VALUE_TYPE: error("Function value cannot add to anything\n");
    default: error("unknown value type passed to valueAdd\n");
  }
}

static int getStringLength(char *format, ...) {
  va_list ap;
  int len = 0;
  va_start(ap, format);
  len = vsnprintf(0, 0, format, ap);
  va_end(ap);
  return len;
}
static int mySnprintf(char *s, int n, char *format, ...){
  va_list ap;
  va_start(ap, format);
  if(n > 0) {
    return vsnprintf(s, n, format, ap);
  } else {
    return vsnprintf(0, 0, format, ap);
  }
  va_end(ap);
}
static int valueToStringInternal(Value* v, char *s, int n) {
  if(!v){
    return mySnprintf(s, n, "none");
  }
  Node* t;
  int i;
  switch(v->type) {
    case INT_TYPE:
      return mySnprintf(s, n, "%d", v->data);
    case FUN_VALUE_TYPE: {
      int len = 0;
      Node* f = ((Closure*) v->data)->f;
      len += mySnprintf(s + len, n - len, "fun %s(", chld(f, 0)->data);
      t = chld(f, 1);
      for(i=0;i<chldNum(t);i++){
        if(i){
          len += mySnprintf(s+len, n-len, ", ");
        }
        len += mySnprintf(s+len, n-len, "%s", chld(t, i)->data);
      }
      len += mySnprintf(s+len, n-len, ")");
      return len;
    }
    case LIST_VALUE_TYPE: {
      int len=0;
      List* l = v->data;
      len += mySnprintf(s+len, n-len, "[");
      for(i=0; i<listSize(l);i++){
        if(i){
          len += mySnprintf(s+len, n-len, ", ");
        }
        len += valueToStringInternal(listGet(l, i), s+len, n-len);
      }
      len += mySnprintf(s+len, n-len, "]");
      return len;
    }
    case STRING_VALUE_TYPE: {
      return mySnprintf(s, n, "%s", v->data);
    }
    default: error("cannot print unknown value type\n");
  }
}
char* valueToString(Value* v) {
  int l = valueToStringInternal(v, 0, 0) + 1;
  char *s = (char*) malloc(l);
  valueToStringInternal(v, s, l);
  return s;
}

void error(char* format,...) {
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
  exit(-1);
}
