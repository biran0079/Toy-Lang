#include "compile.h"
#include "ast.h"
/*

Instruction* newInst(InstType type, int n, ...) {
  Instruction* res = MALLOC(Instruction);
  res->type = type;
  res->data = newList();
  int i;
  va_list v;
  va_start(v, n);
  for(i=0;i<n;i++)
    listPush(res->data, va_arg(v, void*));
  va_end(v);
  return res;
}

Instruction* freeInst(Instruction* ins) {
  freeList(ins->data);
  tlFree(ins);
}

void compileInternal(Node* p, List* res, List* loop) {
  int i,n;
  switch(p->type) {
    case INT_TYPE: listPush(res, newInst(INT_INST, 1, p->data)); break;
    case ID_TYPE: listPush(res, newInst(LOAD_INST, 1, p->data)); break;
    case STMTS_TYPE: {
      n = chldNum(p);
      for(i=0;i<n;i++)
        compileInternal(chld(p, i), res, loop);
      break;
    }
    case ASSIGN_TYPE: {
      compileInternal(chld(p, 1), res, loop);
      Node* ch = chld(p, 0);
      if(ch->type == ID_TYPE)
        listPush(res, newInst(STORE_INST, 1, ch->data));
      else if(ch->type == LIST_ACCESS_TYPE) {
        compileInternal(chld(p, 0), res, loop);
        compileInternal(chld(p, 1), res, loop);
        listPush(res, newInst(LIST_STORE_INST, 1, ch->data));
      } else {
        error("unkonwn node type in first child of assign node: %d\n",
ch->type);
      }
      break;
    }
    case ADD_TYPE: {
      listPush(res, newInst(ADD_INST, 0));
      break;
    }
    case SUB_TYPE: {
      listPush(res, newInst(SUB_INST, 0));
      break;
    }
    case MUL_TYPE: {
      listPush(res, newInst(MUL_INST, 0));
      break;
    }
    case DIV_TYPE: {
      listPush(res, newInst(DIV_INST, 0));
      break;
    }
    case MOD_TYPE: {
      listPush(res, newInst(MOD_INST, 0));
      break;
    }
    case IF_TYPE: {
      compileInternal(chld(p, 0), res, loop);
      Instruction* jmp = newInst(JUMP_ON_FALSE_INST, 1, 0);
      listPush(res, jmp);
      compileInternal(chld(p, 1), res, loop);
      listSet(jmp->data, 0, listSize(res));
      if(chldNum(p) == 3) compileInternal(chld(p, 2), res, loop);
      break;
    }
    case GT_TYPE: {
      listPush(res, newInst(GT_INST, 0));
      break;
    }
    case LT_TYPE: {
      listPush(res, newInst(LT_INST, 0));
      break;
    }
    case GE_TYPE: {
      listPush(res, newInst(GE_INST, 0));
      break;
    }
    case LE_TYPE: {
      listPush(res, newInst(LE_INST, 0));
      break;
    }
    case EQ_TYPE: {
      listPush(res, newInst(EQ_INST, 0));
      break;
    }
    case NE_TYPE: {
      listPush(res, newInst(NE_INST, 0));
      break;
    }
    case AND_TYPE: {
      listPush(res, newInst(AND_INST, 0));
      break;
    }
    case OR_TYPE: {
      listPush(res, newInst(OR_INST, 0));
      break;
    }
    case NOT_TYPE: {
      listPush(res, newInst(NOT_INST, 0));
      break;
    }
    case WHILE_TYPE: {
      int pos0 = listSize(res);
      compileInternal(chld(p, 0), res, loop);
      listPush(loop, pos0);
      listPush(loop, listSize(res));
      Instruction* jmp1 = newInst(JUMP_ON_FALSE_INST, 1, 0);
      listPush(res, jmp1);
      compileInternal(chld(p, 1), res, loop);
      listPop(loop);
      listPop(loop);
      Instruction* jmp2 = newInst(GOTO, 1, pos0);
      listPush(res, jmp2);
      listSet(jmp1->data, 0, listSize(res));
      break;
    }
    case FUN_TYPE: {
      long id = (long) chld(p, 0)->data;
      listPush(res, newInst(CLOSURE_INST, 1, listSize(res)+3)));
      listPush(res, newInst(STORE_INST, 1, id));
      Instruction* gotoInst = newInst(GOTO_INST, 1, 0);
      listPush(res, gotoInst);
      Node* argLst = chld(p, 1);
      for(i = chldNum(argLst)-1; i >= 0; i--) {
        listPush(res, newInst(STORE_INST, 1, chld(argLst, i)->data));
      }
      List* newLoop = newList();
      compileInternal(chld(p, 2), res, newLoop);
      freeList(newLoop);
      listSet(gotoInst->data, 0, listSize(res));
      break;
    }
    case EXP_LIST_TYPE: {
      for(i=0; i<chldNum(argLst); i++) {
        compileInternal(chld(p, i), res, loop);
      }
      break;
    }
    case TAIL_CALL_TYPE:
    case CALL_TYPE: {
      compileInternal(chld(p, 1), res, loop);
      compileInternal(chld(p, 0), res, loop);
      listPush(res, newInst(CALL_INST, 0));
      break;
    }
    case RETURN_TYPE: {
      listPush(res, newInst(RETURN_INST, 0));
      break;
    }
    case BREAK_TYPE: {
      listPush(res, newInst(INT_INST, 1, 0));
      listPush(res, newInst(GOTO, listLast(loop)));
      break;
    }
    case CONTINUE_TYPE: {
      listPush(res, newInst(GOTO, listGet(loop, listSize(loop)-2)));
      break;
    }
    case LIST_TYPE: {
      for(i=0;i<chldNum(p);i++) {
        compileInternal(chld(p, i), res, loop);
      }
      listPush(res, newInst(LIST_INST, 1, chldNum(p)));
      break;
    }
    case LIST_ACCESS_TYPE: {
      compileInternal(chld(p, 0), res, loop);
      compileInternal(chld(p, 1), res, loop);
      listPush(res, newInst(LIST_ACCESS_INST, 0));
      break;
    }
    case ADDEQ_TYPE: {
      compileInternal(chld(p, 0), res, loop);
      compileInternal(chld(p, 1), res, loop);
      listPush(res, newInst(ADDEQ_INST, 0));
      break;
    }
    case TIME_TYPE: {
      listPush(res, newInst(TIME_START_INST, 0));
      compileInternal(chld(p, 0), res, loop);
      listPush(res, newInst(TIME_END_INST, 0));
      break;
    }
    case STRING_TYPE: {
      listPush(res, newInst(STRING_INST, 1, p->data));
      break;
    }
    case NONE_TYPE: {
      listPush(res, newInst(NONE_INST, 0));
      break;
    }
    case TRY_TYPE: {
      Instruction* try = newInst(TRY_INST, 0);
      listPush(res, try);
      compileInternal(chld(p, 0), res, loop);
      Instruction* jmp = newInst(GOTO_INST, 0);
      listPush(res, jmp);
      listPush(try->data, listSize(res));
      compileInternal(chld(p, 1), res, loop);
      listPush(jmp->data, listSize(res));
      if (chldNum(p) == 3) {
        listPush(try->data, listSize(res));
        compileInternal(chld(p, 2), res, loop);
      }
      break;
    }
    case THROW_TYPE: {
      compileInternal(chld(p, 0), res, loop);
      listPush(res, newInst(THROW_INST, 0));
      break;
    }
    case LOCAL_TYPE: {
      n = chldNum(p);
      for(i=0;i<n;i++) {
        listPush(res, newInst(NONE_INST, 0));
        listPush(res, newInst(STORE_INST, 1, chld(p, i)->data));
      }
      break;
    }
    case ADDADD_TYPE: {

    }
    case FOR_TYPE: {
    }
    case FOREACH_TYPE: {
    }
    case IMPORT_TYPE: {

    }
    case MODULE_ACCESS_TYPE: {

    }
  }
}

List* compile(Node* p) {
  List* res = newList();
  List* loop = newList();
  compileInternal(p, res, loop);
  freeList(loop);
  return res;
}
*/
