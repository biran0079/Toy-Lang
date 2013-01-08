#include "tl.h"
#include "ast.h"
#include "value.h"
#include "env.h"
#include "closure.h"
#include "list.h"
#include "hashTable.h"
#include "eval.h"
#include "util.h"

Value* eval(Value* ev, Node* p) {
  Env* e = ev->data;
  Value* res = 0;
  int i;
  switch(p->type) {
    case STMTS_TYPE: {
      List* l = (List*) p->data;
      for(i=0;i<listSize(l);i++){
        eval(ev, listGet(l,i));
      }
      return newNoneValue();
    }
    case EXP_LIST_TYPE: {
      Value* res = 0;
      for(i=0;i<chldNum(p);i++)
        res = eval(ev, chld(p, i));
      return res;
    }
    case PRINT_TYPE: {
      Node* exps = chld(p,0);
      for(i=0;i<chldNum(exps);i++) {
        if(i)putchar(' ');
        printf("%s", valueToString(eval(ev, chld(exps, i))));
      }
      printf("\n");
      return newNoneValue();
    }
    case LEN_TYPE: {
      Value* v = eval(ev, chld(p, 0));
      switch(v->type) {
        case LIST_VALUE_TYPE: return newIntValue(listSize((List*) v->data));
        case STRING_VALUE_TYPE: return newIntValue(strlen((char*) v->data));
        default: error("unsupported type for 'len' operator\n");
      }
      error("should never reach here");
    }
    case STR_TYPE: {
      return newStringValue(valueToString(eval(ev, chld(p,0))));
    }
    case ORD_TYPE: {
      Value* v = eval(ev, chld(p, 0));
      if(v->type != STRING_VALUE_TYPE || strlen((char*) v->data) != 1)
        error("ord() can only apply to string with length 1\n");
      return newIntValue(*((char*) v->data));
    }
    case NONE_TYPE: return newNoneValue();
    case LIST_TYPE: {
      List* vs = newList();
      for(i=0;i<chldNum(p);i++)
        listPush(vs, eval(ev, chld(p,i)));
      return newListValue(vs);
    }
    case LIST_ACCESS_TYPE:{
      Value* v = eval(ev, chld(p, 0));
      Value* idxValue = eval(ev, chld(p, 1));
      if(idxValue->type != INT_VALUE_TYPE) error("list index must be int\n");
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
    case TAIL_CALL_TYPE: {
      Value* closureValue = envGet(e, chld(p, 0)->data);
      if(closureValue->type != CLOSURE_VALUE_TYPE) error("only closure value can be called\n");
      Closure *c = closureValue->data;
      Node* f = c->f, *args = chld(p, 1);
      Env* e2 = newEnv(c->e->data);
      Value* ev2 = newEnvValue(e2);
      Node *ids = chld(f,1);
      if(chldNum(ids) != chldNum(args)) error("function parameter number incorrecr\n");
      for(i=0; i<chldNum(ids); i++)
        envPutLocal(e2, chld(ids, i)->data, eval(ev, chld(args, i)));
      e->tailCall = newClosureValue(f, ev2);
      longjmp(e->retState, 2);
      // actuall call is handled by CALL_TYPE
    }
    case CALL_TYPE: {
      Value* closureValue = envGet(e, chld(p, 0)->data);
      if(!closureValue) error("fun value is none in function application\n");
      Closure *c = closureValue->data;
      Node* f = c->f, *args = chld(p, 1);
      Env* e2 = newEnv(c->e->data);
      Value* ev2 = newEnvValue(e2);
      Node *ids = chld(f,1);
      if(chldNum(ids)!=chldNum(args)) error("function parameter number incorrecr\n");
      for(i=0; i<chldNum(ids); i++)
        envPutLocal(e2, chld(ids, i)->data, eval(ev, chld(args, i)));
      while(1){
        int ret = setjmp(e2->retState);
        if(!ret){
          eval(ev2, chld(f, 2));
          // if reach here, no return statement is called, so none is returned 
          return newNoneValue();
        } else if(ret == 1) {
          // return is called;
          Value* returnValue = e2->returnValue;
          return returnValue;
        } if(ret == 2) {
          // tail recursive call, reuse environment e2
          Closure* tc = e2->tailCall->data;
          f = tc->f;
          ev2 = tc->e;
          e2 = ev2->data;
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
      e->returnValue = eval(ev, chld(p, 0));
      longjmp(e->retState, 1);
    }
    case ID_TYPE: {
      Value* res = envGet(e, (char*) p->data);
      return res ? res : newNoneValue();
    }
    case INT_TYPE:
      return newIntValue((long) p->data);
    case STRING_TYPE:
      return newStringValue((char*) p->data);
    case ASSIGN_TYPE: {
      Node* left = chld(p, 0);
      switch(left->type) {
        case LIST_ACCESS_TYPE: {
          Value* lv = eval(ev, chld(left, 0));
          if (lv->type != LIST_VALUE_TYPE) error("= only applys to list\n");
          List* l = lv->data;                      
          Value* idx = eval(ev, chld(left, 1));
          if(idx->type != INT_VALUE_TYPE) error("list index must be int\n");
          listSet(l, (long) idx->data, eval(ev, chld(p, 1)));
          return listGet(l, (long) idx->data);
        }
        case ID_TYPE: {
          Value* res = eval(ev, chld(p, 1));
          envPut(e, (char*) left->data, res);
          return res;
        }
        default: error("left hand side of = must be a left value\n");
      }
    }
    case ADDEQ_TYPE: {
      Node* left = chld(p,0);
      switch(left->type) {
        case LIST_ACCESS_TYPE: {
          Value* lv = eval(ev, chld(left, 0));
          if(lv->type != LIST_VALUE_TYPE) error("+= only applys to list\n");
          List* l = lv->data;                      
          Value* idx = eval(ev, chld(left, 1));
          if(idx->type != INT_VALUE_TYPE) error("list index must be int\n");
          Value* e1 = listGet(l, (long) idx->data);
          Value* e2 = eval(ev, chld(p, 1));
          Value* res = newNoneValue();
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
              res = e1;
              break;
            default: error("unknown type for operator +=: %d\n", e1->type);
          }
          return res;
        }
        case ID_TYPE: {
          char* key = (char *) chld(p, 0)->data;
          Value* e1 = envGet(e, key);
          Value* e2 = eval(ev, chld(p, 1));
          Value* res = newNoneValue();
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
              res = e1;
              break;
            default: error("unknown type for operator +=\n");
          }
          return res;
        }
        default: error("left hand side of += must be left value\n");
      }
    }
    case ADD_TYPE:
      return valueAdd(eval(ev, chld(p, 0)), eval(ev, chld(p, 1)));
    case SUB_TYPE:
      return valueSub(eval(ev, chld(p, 0)), eval(ev, chld(p, 1)));
    case MUL_TYPE:
      return valueMul(eval(ev, chld(p, 0)), eval(ev, chld(p, 1)));
    case DIV_TYPE: 
      return valueDiv(eval(ev, chld(p, 0)), eval(ev, chld(p, 1)));
    case MOD_TYPE: 
      return valueMod(eval(ev, chld(p, 0)), eval(ev, chld(p, 1)));
    case IF_TYPE:
      if(eval(ev, chld(p, 0))->data)
        eval(ev, chld(p, 1));
      else if(chldNum(p)==3)
        eval(ev, chld(p, 2));
      return 0;
    case FOR_TYPE: {
      jmp_buf buf;
      listPush(e->loopStates, buf);
      for(eval(ev, chld(p,0)); eval(ev, chld(p, 1))->data; eval(ev, chld(p, 2))){
          int jmp = setjmp(buf);
          if(jmp==0){
            // regular case
            eval(ev, chld(p, 3));
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
      return newNoneValue();
    }
    case FOREACH_TYPE:{
      jmp_buf buf;
      listPush(e->loopStates, buf);
      Node* id = chld(p, 0);
      Value* lv = eval(ev, chld(p, 1));
      if(id->type != ID_TYPE || lv->type != LIST_VALUE_TYPE) {
        error("param type incorrect for for( : ) statement\n ");
      }
      List* l = (List*) lv->data;
      int len = listSize(l);
      for(i=0;i<len;i++) {
        envPut(e, (char*) id->data, listGet(l, i));
        int jmp = setjmp(buf);
        if(jmp==0) {
          eval(ev, chld(p, 2));
        } else if(jmp==1) {
          continue;
        } else if(jmp==2) {
          break;
        } else {
          error("unknown longjmp state\n");
        }
      }
      listPop(e->loopStates);
      return newNoneValue();
    }
    case WHILE_TYPE:
      {
        jmp_buf buf;
        listPush(e->loopStates, buf);
        while(eval(ev, chld(p, 0))->data){
          int jmp = setjmp(buf);
          if(jmp==0) {
            // regular case 
            eval(ev, chld(p, 1));
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
        return newNoneValue();
      }
    case CONTINUE_TYPE:
      longjmp(listLast(e->loopStates), 1);
    case BREAK_TYPE:
      longjmp(listLast(e->loopStates), 2);
    case GT_TYPE:
      return newIntValue(
           (long) eval(ev, chld(p, 0))->data > (long) eval(ev, chld(p, 1))->data);
    case LT_TYPE:
      return newIntValue(
           (long) eval(ev, chld(p, 0))->data < (long) eval(ev, chld(p, 1))->data);
    case GE_TYPE:
      return newIntValue(
           (long) eval(ev, chld(p, 0))->data >= (long) eval(ev, chld(p, 1))->data);
    case LE_TYPE:
      return newIntValue(
           (long) eval(ev, chld(p, 0))->data <= (long) eval(ev, chld(p, 1))->data);
    case EQ_TYPE:
      return newIntValue(
           valueEquals(eval(ev, chld(p, 0)), eval(ev, chld(p, 1))));
    case NE_TYPE:
      return newIntValue(
           !valueEquals(eval(ev, chld(p, 0)), eval(ev, chld(p, 1))));
    case AND_TYPE:
      return newIntValue(
           eval(ev, chld(p, 0))->data && eval(ev, chld(p, 1))->data);
    case OR_TYPE:
      return newIntValue(
           eval(ev, chld(p, 0))->data || eval(ev, chld(p, 1))->data);
    case NOT_TYPE:
      return newIntValue(! eval(ev, chld(p, 0))->data);
    case FUN_TYPE:
      envPut(e, (char*) chld(p, 0)->data, newClosureValue(p, ev));
      return envGet(e, (char*) chld(p, 0)->data);
    case TIME_TYPE: {
      clock_t st = clock();                
      Value* res = eval(ev, chld(p, 0));
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
          eval(ev, tryBlock);
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
          eval(ev, catchBlock);
          break;
        }
        default: error("unknown state passed from longjmp in try statement\n");
                 break;
      }
      if(finallyBlock) eval(ev, finallyBlock);
      return newNoneValue();
    }
    case THROW_TYPE: {
      Value* v = eval(ev, chld(p, 0));           
      throwValue(e, v);
    }
    case ADDADD_TYPE: {
      // i++ type                  
      Node* left = chld(p, 0);
      switch(left->type) {
        case ID_TYPE: {
          char* id = (char*) left->data;
          Value* i = envGet(e, id);
          if(i->type != INT_VALUE_TYPE) error("++ can only apply to int\n");
          envPut(e, id, newIntValue((long) i->data + 1));
          return i;
        }
        case LIST_ACCESS_TYPE: {
          Value* lv =  eval(ev, chld(left, 0));
          if(lv->type != LIST_VALUE_TYPE) error("++ only applys to left value\n");
          List* l = (List*) lv->data;
          Value* idx = eval(ev, chld(left, 1));
          if(idx->type != INT_VALUE_TYPE) error("index of list must be int\n");
          long idxv = (long) idx->data;
          Value* i = listGet(l, idxv);
          if (i->type != INT_VALUE_TYPE) error("++ only applys on int\n");
          listSet(l, idxv, newIntValue((long) i->data + 1));
          return i;
        }
      } 
    }
    case LOCAL_TYPE: {
      Node *ids = chld(p, 0);
      for(i=0;i<chldNum(ids);i++) {
        envPutLocal(e, (char*) chld(ids, i)->data, newNoneValue());
      }
      return 0;
    }
    default:
      error("cannot eval unknown node type\n");
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

