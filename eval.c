#include "tl.h"
#include "ast.h"
#include "value.h"
#include "env.h"
#include "execUnit.h"
#include "exception.h"
#include "closure.h"
#include "list.h"
#include "hashTable.h"
#include "eval.h"
#include "tljmp.h"
#include "util.h"

extern List* rootValues, *parseTrees;
extern JmpMsg __jmpMsg__;
extern Value* globalEnv;

void pushRootValue(Value* v) {
  listPush(rootValues, v);
}

static void popRootValueTo(int size) {
  if(size > rootValues->size) error("size larger than rootValues->size\n");
  rootValues->size = size;
}

static Value* evalAndPushRoot(Value* ev, Node* p) {
  Value* res = eval(ev, p);
  pushRootValue(res);
  return res;
}

static Value* evalBuiltinFun(Value* e, Node* exp_lst, BuiltinFun f) {
  List* args = newList();
  int i, n = chldNum(exp_lst);
  for(i=0; i<n; i++) {
    Value* v = evalAndPushRoot(e, chld(exp_lst, i));
    listPush(args, v);
  }
  Value* res = f(args);
  freeList(args);
  return res;
}

Value* eval(Value* ev, Node* p) {
  int initSize = listSize(rootValues);

  Env* e = ev->data;
  Value* res = newNoneValue();
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
      Value* res = newNoneValue();
      for(i=0;i<chldNum(p);i++)
        res = eval(ev, chld(p, i));
      return res;
    }
    case NONE_TYPE: return newNoneValue();
    case LIST_TYPE: {
      List* vs = newList();
      for(i=0;i<chldNum(p);i++) {
        Value* v = evalAndPushRoot(ev, chld(p,i));
        listPush(vs, v);
      }
      Value* res = newListValue(vs);
      popRootValueTo(initSize);
      return res;
    }
    case LIST_ACCESS_TYPE:{
      Value* v = evalAndPushRoot(ev, chld(p, 0));
      Value* idxValue = evalAndPushRoot(ev, chld(p, 1));
      if(idxValue->type != INT_VALUE_TYPE) error("list index must be int\n");
      long idx = (long) idxValue->data;
      Value *res;
      switch(v->type) {
        case LIST_VALUE_TYPE: {
          List* l = (List*) v->data;                      
          res = listGet(l, idx);
          break;
        }
        case STRING_VALUE_TYPE: {
          char *s = (char*) v->data;
          char *ss = (char*) tlMalloc(2);
          ss[0] = s[idx];
          ss[1] = 0;
          res = newStringValue(ss);
          break;
        }
      }
      popRootValueTo(initSize);
      return res;
    }
    case TAIL_CALL_TYPE: {
      Value* closureValue = evalAndPushRoot(ev, chld(p, 0));
      if(closureValue->type == BUILTIN_FUN_VALUE_TYPE) {
        Value* res = evalBuiltinFun(ev, chld(p, 1), closureValue->data);
        popRootValueTo(initSize);
        return res;
      }
      if(closureValue->type != CLOSURE_VALUE_TYPE) error("only closure value can be called\n");
      Closure *c = closureValue->data;
      Node* f = c->f, *args = chld(p, 1);
      Env* e2 = newEnv(c->e);
      Value* ev2 = newEnvValue(e2);
      pushRootValue(ev2);
      Node *ids = chld(f,1);
      if(chldNum(ids) != chldNum(args))
        error("%s parameter number incorrect\n", valueToString(closureValue));
      for(i=0; i<chldNum(ids); i++)
        envPutLocal(e2, (long) chld(ids, i)->data, eval(ev, chld(args, i)));
      Value* res = newClosureValue(f, ev2);
      tlLongjmp(e->retState, TAIL_CALL_MSG_TYPE, res);
      // actuall call is handled by CALL_TYPE
    }
    case CALL_TYPE: {
      Value* closureValue = evalAndPushRoot(ev, chld(p, 0));
      if(closureValue->type == BUILTIN_FUN_VALUE_TYPE) {
        Value* res = evalBuiltinFun(ev, chld(p, 1), closureValue->data);
        popRootValueTo(initSize);
        return res;
      }
      if(!closureValue) error("fun value is none in function application\n");
      Closure *c = closureValue->data;
      Node* f = c->f, *args = chld(p, 1);
      Env* e2 = newEnv(c->e);
      Value* ev2 = newEnvValue(e2);
      pushRootValue(ev2);
      Node *ids = chld(f,1);
      if(chldNum(ids)!=chldNum(args))
        error("%s parameter number incorrect\n", valueToString(closureValue));
      for(i=0; i<chldNum(ids); i++)
        envPutLocal(e2, (long) chld(ids, i)->data, eval(ev, chld(args, i)));
      while(1){
        int loopN = envNumOfLoopStates(e), exN = envNumOfExceptionStates(e);
        int ret = setjmp(e2->retState);
        if(!ret){
          eval(ev2, chld(f, 2));
          // if reach here, no return statement is called, so none is returned 
          popRootValueTo(initSize);
          return newNoneValue();
        } else{
          envRestoreStates(e, loopN, exN);
          if(__jmpMsg__.type == RETURN_MSG_TYPE) {
            // return is called;
            Value* returnValue = __jmpMsg__.data;
            popRootValueTo(initSize);
            return returnValue;
          } if(__jmpMsg__.type == TAIL_CALL_MSG_TYPE) {
            // tail recursive call, reuse environment e2
            Value* closureValue = __jmpMsg__.data;
            Closure* tc = closureValue->data;
            f = tc->f;
            ev2 = tc->e;
            e2 = ev2->data;
            popRootValueTo(initSize);
            pushRootValue(ev2); // ev2 is all we need to keep
            continue;
          } if(__jmpMsg__.type == EXCEPTION_MSG_TYPE) {
            // exception passed out
            Value* v = __jmpMsg__.data;
            popRootValueTo(initSize);
            throwValue(e, v);
          } else {
            error("unknown  return value from longjmp in function call\n");
          }
        }
      }
    }
    case RETURN_TYPE:{
      if(e->parent == newNoneValue()) error("cannot call return outside a function.\n");
      tlLongjmp(e->retState, RETURN_MSG_TYPE, eval(ev, chld(p, 0)));
    }
    case ID_TYPE: {
      Value* res = envGet(e, (long) p->data);
      return res ? res : newNoneValue();
    }
    case INT_TYPE:
      return newIntValue((long) p->data);
    case STRING_TYPE:
      return newStringValue(copyStr(p->data));
    case ASSIGN_TYPE: {
      Node* left = chld(p, 0);
      switch(left->type) {
        case LIST_ACCESS_TYPE: {
          Value* lv = evalAndPushRoot(ev, chld(left, 0));
          if (lv->type != LIST_VALUE_TYPE) error("= only applys to list\n");
          List* l = lv->data;                      
          Value* idx = evalAndPushRoot(ev, chld(left, 1));
          if(idx->type != INT_VALUE_TYPE) error("list index must be int\n");
          listSet(l, (long) idx->data, eval(ev, chld(p, 1)));
          Value* res = listGet(l, (long) idx->data);
          popRootValueTo(initSize);
          return res;
        }
        case ID_TYPE: {
            Value* res = eval(ev, chld(p, 1));
          envPut(e, (long) left->data, res);
          return res;
        }
        case MODULE_ACCESS_TYPE: {
          Value* res = eval(ev, chld(p, 1));
          int n = chldNum(left);
          Value* env = ev;
          for(i=0; i < n - 1; i++) {
            long id = (long) chld(left, i)->data;
            if(env->type != ENV_VALUE_TYPE) error("module access must use env type, %s get\n", valueToString(env));
            env = envGet(env->data, id);
          }
          envPutLocal(env->data, (long) chld(left, i)->data, res);
          return res;
        }
        default: error("left hand side of = must be a left value\n");
      }
    }
    case ADDEQ_TYPE: {
      Node* left = chld(p,0);
      switch(left->type) {
        case LIST_ACCESS_TYPE: {
          Value* lv = evalAndPushRoot(ev, chld(left, 0));
          if(lv->type != LIST_VALUE_TYPE) error("+= only applys to list\n");
          List* l = lv->data;                      
          Value* idx = evalAndPushRoot(ev, chld(left, 1));
          if(idx->type != INT_VALUE_TYPE) error("list index must be int\n");
          Value* e1 = listGet(l, (long) idx->data);
          Value* e2 = evalAndPushRoot(ev, chld(p, 1));
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
          popRootValueTo(initSize);
          return res;
        }
        case ID_TYPE: {
          long key = (long) chld(p, 0)->data;
          Value* e1 = envGet(e, key);
          Value* e2 = evalAndPushRoot(ev, chld(p, 1));
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
          popRootValueTo(initSize);
          return res;
        }
        default: error("left hand side of += must be left value\n");
      }
    }
    case ADD_TYPE: {
      Value* e1 = evalAndPushRoot(ev, chld(p, 0));
      Value* e2 = evalAndPushRoot(ev, chld(p, 1));
      Value* res = valueAdd(e1, e2);
      popRootValueTo(initSize);
      return res;
    }
    case SUB_TYPE: {
      Value*e1 = evalAndPushRoot(ev, chld(p, 0));
      Value*e2 = evalAndPushRoot(ev, chld(p, 1));
      Value*res = valueSub(e1, e2);
      popRootValueTo(initSize);
      return res;
    }
    case MUL_TYPE: {
      Value*e1 = evalAndPushRoot(ev, chld(p, 0));
      Value*e2 = evalAndPushRoot(ev, chld(p, 1));
      Value*res = valueMul(e1, e2);
      popRootValueTo(initSize);
      return res;
    }
    case DIV_TYPE: {
      Value*e1 = evalAndPushRoot(ev, chld(p, 0));
      Value*e2 = evalAndPushRoot(ev, chld(p, 1));
      Value*res = valueDiv(e1, e2);
      popRootValueTo(initSize);
      return res;
    }
    case MOD_TYPE: {
      Value*e1 = evalAndPushRoot(ev, chld(p, 0));
      Value*e2 = evalAndPushRoot(ev, chld(p, 1));
      Value*res = valueMod(e1, e2);
      popRootValueTo(initSize);
      return res;
    }
    case IF_TYPE:
      if(eval(ev, chld(p, 0))->data)
        eval(ev, chld(p, 1));
      else if(chldNum(p)==3)
        eval(ev, chld(p, 2));
      return newNoneValue();
    case FOR_TYPE: {
      jmp_buf buf;
      listPush(envGetLoopStates(e), buf);
      for(eval(ev, chld(p,0)); eval(ev, chld(p, 1))->data; eval(ev, chld(p, 2))){
          int loopN = envNumOfLoopStates(e), exN = envNumOfExceptionStates(e);
          if(setjmp(buf)==0) {
            eval(ev, chld(p, 3));
          } else {
            envRestoreStates(e, loopN, exN);
            JmpMsg* msg = &__jmpMsg__;
            if(msg->type == CONTINUE_MSG_TYPE)
              continue;
            else if(msg->type == BREAK_MSG_TYPE)
              break;
            else
              error("unexpected jmpmsg type: %d\n", msg->type);
          }
      }
      listPop(envGetLoopStates(e));
      return newNoneValue();
    }
    case FOREACH_TYPE:{
      jmp_buf buf;
      listPush(envGetLoopStates(e), buf);
      Node* id = chld(p, 0);
      Value* lv = evalAndPushRoot(ev, chld(p, 1));
      if(id->type != ID_TYPE || lv->type != LIST_VALUE_TYPE) {
        error("param type incorrect for for( : ) statement\n ");
      }
      List* l = (List*) lv->data;
      int len = listSize(l);
      for(i=0;i<len;i++) {
        envPut(e, (long) id->data, listGet(l, i));
        int loopN = envNumOfLoopStates(e), exN = envNumOfExceptionStates(e);
        if(setjmp(buf)==0) {
          eval(ev, chld(p, 2));
        } else {
          envRestoreStates(e, loopN, exN);
          JmpMsg* msg = &__jmpMsg__;
          if(msg->type == CONTINUE_MSG_TYPE)
            continue;
          else if(msg->type == BREAK_MSG_TYPE)
            break;
          else
            error("unexpected jmpmsg type: %d\n", msg->type);
        }
      }
      listPop(envGetLoopStates(e));
      popRootValueTo(initSize);
      return newNoneValue();
    }
    case WHILE_TYPE:
      {
        jmp_buf buf;
        listPush(envGetLoopStates(e), buf);
        while(eval(ev, chld(p, 0))->data){
          int loopN = envNumOfLoopStates(e), exN = envNumOfExceptionStates(e);
          if(setjmp(buf)==0) {
            eval(ev, chld(p, 1));
          } else {
            envRestoreStates(e, loopN, exN);
            JmpMsg* msg = &__jmpMsg__;
            if(msg->type == CONTINUE_MSG_TYPE)
              continue;
            else if(msg->type == BREAK_MSG_TYPE){
              break;
            }
            else
              error("unexpected jmpmsg type: %d\n", msg->type);
          }
        }
        listPop(envGetLoopStates(e));
        return newNoneValue();
      }
    case CONTINUE_TYPE:
      tlLongjmp(listLast(envGetLoopStates(e)), CONTINUE_MSG_TYPE, 0);
    case BREAK_TYPE:
      tlLongjmp(listLast(envGetLoopStates(e)), BREAK_MSG_TYPE, 0);
    case GT_TYPE:
      return newIntValue(
           valueCmp(evalAndPushRoot(ev, chld(p, 0)), evalAndPushRoot(ev, chld(p, 1))) > 0);
    case LT_TYPE:
      return newIntValue(
           valueCmp(evalAndPushRoot(ev, chld(p, 0)), evalAndPushRoot(ev, chld(p, 1))) < 0);
    case GE_TYPE:
      return newIntValue(
           valueCmp(evalAndPushRoot(ev, chld(p, 0)), evalAndPushRoot(ev, chld(p, 1))) >= 0);
    case LE_TYPE:
      return newIntValue(
           valueCmp(evalAndPushRoot(ev, chld(p, 0)), evalAndPushRoot(ev, chld(p, 1))) <= 0);
    case EQ_TYPE: 
      return newIntValue(
           valueCmp(evalAndPushRoot(ev, chld(p, 0)), evalAndPushRoot(ev, chld(p, 1))) == 0);
    case NE_TYPE: 
      return newIntValue(
           !valueEquals(evalAndPushRoot(ev, chld(p, 0)), evalAndPushRoot(ev, chld(p, 1))));
    case AND_TYPE:
      return newIntValue(
           eval(ev, chld(p, 0))->data && eval(ev, chld(p, 1))->data);
    case OR_TYPE:
      return newIntValue(
           eval(ev, chld(p, 0))->data || eval(ev, chld(p, 1))->data);
    case NOT_TYPE:
      return newIntValue(! eval(ev, chld(p, 0))->data);
    case FUN_TYPE: {
      Value* res = newClosureValue(p, ev);
      envPut(e, (long) chld(p, 0)->data, res);
      return res;
    }
    case TIME_TYPE: {
      clock_t st = clock();                
      Value* res = eval(ev, chld(p, 0));
      fprintf(stderr, "time: %lf secs\n", (clock() - st) * 1.0 / CLOCKS_PER_SEC);
      return newNoneValue();
    }
    case TRY_TYPE: {
      ExecUnit* finally= chldNum(p) == 4 ? newExecUnit(ev, chld(p, 3)) : 0;
      Exception* ex = newException(finally);
      envPushExceptionStates(e, ex);
      Node* tryBlock = chld(p, 0);
      long catchId = (long) chld(p, 1)->data;
      Node* catchBlock = chld(p, 2);
      int loopN = envNumOfLoopStates(e), exN = envNumOfExceptionStates(e);
      if(setjmp(ex->buf) == 0) {
        // try block
        eval(ev, tryBlock);
      } else {
        // catch block
        envRestoreStates(e, loopN, exN);
        if(setjmp(ex->buf) == 0) {
          Value* v = __jmpMsg__.data;
          envPutLocal(e, catchId, v);
          eval(ev, catchBlock);
        } else {
          // exception from catch or finally block
          Value* v = __jmpMsg__.data;
          pushRootValue(v);
          envRestoreStates(e, loopN, exN);
          envPopExceptionStates(e);
          throwValue(e, v);
        }
      }
      envPopExceptionStates(e);
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
          long id = (long) left->data;
          Value* i = envGet(e, id);
          if(i->type != INT_VALUE_TYPE) error("++ can only apply to int\n%s\n", valueToString(nodeToListValue(p)));
          envPut(e, id, newIntValue((long) i->data + 1));
          return i;
        }
        case LIST_ACCESS_TYPE: {
          Value* lv =  evalAndPushRoot(ev, chld(left, 0));
          if(lv->type != LIST_VALUE_TYPE) error("++ only applys to left value\n%s\n", valueToString(nodeToListValue(p)));
          List* l = (List*) lv->data;
          Value* idx = evalAndPushRoot(ev, chld(left, 1));
          if(idx->type != INT_VALUE_TYPE) error("index of list must be int\n");
          long idxv = (long) idx->data;
          Value* i = listGet(l, idxv);
          if (i->type != INT_VALUE_TYPE) error("++ only applys on int\n");
          listSet(l, idxv, newIntValue((long) i->data + 1));
          popRootValueTo(initSize);
          return i;
        }
      } 
    }
    case LOCAL_TYPE: {
      Node *ids = chld(p, 0);
      for(i=0;i<chldNum(ids);i++) {
        envPutLocal(e, (long) chld(ids, i)->data, newNoneValue());
      }
      return newNoneValue();
    }
    case IMPORT_TYPE: {
      Node* id = chld(p, 0);
      char *s = catStr(getStrId((long) id->data), ".tl");
      FILE* f = openFromPath(s, "r");
      if(!f) ("cannot open file %s\n", s);
      yyrestart(f);
      if(yyparse()) error("failed to parse %s\n", s);
      tlFree(s);
      Value* res = newEnvValue(newEnv(globalEnv));
      pushRootValue(res);
      eval(res, listLast(parseTrees));
      envPut(e, (long) id->data, res);
      popRootValueTo(initSize);
      return newNoneValue();
    }
    case MODULE_ACCESS_TYPE: {
      int n = chldNum(p);
      Value* res = ev;
      for(i=0; i<n; i++) {
        long id = (long) chld(p, i)->data;
        if(res->type != ENV_VALUE_TYPE) error("module access must use env type, %s get\n", valueToString(res));
        res = envGet(res->data, id);
      }
      return res;
    }
    default:
      error("cannot eval unknown node type\n");
  }
  error("should not reach here\n");
}

void throwValue(Env* e, Value* v) {
  if(envLastExceptionState(e)) {
    // currently in try block
    tlLongjmp(envLastExceptionState(e)->buf, EXCEPTION_MSG_TYPE, v);
  } else {
    // If has parent stack frame, pass to it. Report error otherwise
    if (e->parent != newNoneValue()) {
      tlLongjmp(e->retState, EXCEPTION_MSG_TYPE, v);
    } else {
      error("uncaught exception: %s\n", valueToString(v));
    }
  }
}

