#include "compile.h"
#include "ast.h"

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

void compileInternal(Node* p, List* res) {
  int i,n;
  switch(p->type) {
                     /*
    case INT_TYPE: listPush(res, newInst(INT_INST, 1, p->data)); break;
    case ID_TYPE: listPush(res, newInst(LOAD_INST, 1, p->data)); break;
    case STMTS_TYPE: {
      n = chldNum(p);
      for(i=0;i<n;i++)
        compileInternal(chld(p, i), res);
      break;
    }
    case ASSIGN_TYPE: {
      compileInternal(chld(p, 1), res);
      Node* ch = chld(p, 0);
      if(ch->type == ID_TYPE)
        listPush(res, newInst(STORE_INST, 1, ch->data));
      else if(ch->type == LIST_ACCESS_TYPE) {
        compileInternal(chld(p, 0), res);
        compileInternal(chld(p, 1), res);
        listPush(res, newInst(LIST_STORE_INST, 1, ch->data));
      } else {
        error("unkonwn node type in first child of assign node: %d\n", ch->type);
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
    case IF_TYPE: {
      compileInternal(chld(p, 0), res);
      Instruction* jmp = newInst(JUMP_ON_FALSE_INST, 1, 0);
      listPush(res, jmp);
      compileInternal(chld(p, 1), res);
      listSet(jmp->data, 0, listSize(res));
      if(chldNum(p) == 3) compileInternal(chld(p, 2), res);
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
      
    }
    FUN_TYPE,
    EXP_LIST_TYPE,
    ID_LIST_TYPE,
    CALL_TYPE,
    TAIL_CALL_TYPE,
    RETURN_TYPE,
    BREAK_TYPE,
    CONTINUE_TYPE,
    MOD_TYPE,
    LIST_TYPE,
    LIST_ACCESS_TYPE,
    FOR_TYPE,
    ADDEQ_TYPE,
    TIME_TYPE,
    STRING_TYPE, 
    NONE_TYPE,
    FOREACH_TYPE,
    TRY_TYPE,
    THROW_TYPE,
    ADDADD_TYPE,
    LOCAL_TYPE,
    IMPORT_TYPE,
    MODULE_ACCESS_TYPE,
    */
  }
}

List* compile(Node* p) {
  List* res = newList();
  compileInternal(p, res);
  return res;
}
