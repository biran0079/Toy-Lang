#include "value.h"
#include "ast.h"
#include "util.h"
#include "list.h"

int newNodeC = 0, freeNodeC = 0;

Node* newNode(NodeType t, void* i) {
  newNodeC++;
  Node* res = MALLOC(Node);
  res->type = t;
  res->data = i;
  res->eval = 0;
  return res;
}

Node* newNode2(NodeType t, int n, ... ) {
  newNodeC++;
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
  res->eval = 0;
  va_end(v);
  return res;
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

Value* nodeToListValue(Node* p) {
  List* l = newList();
  Value* res = newListValue(l);
  pushRootValue(res);
  listPush(l, newStringValue(copyStr(nodeTypeToString(p->type))));
  switch(p->type){
    case INT_TYPE: listPush(l, newIntValue((long) p->data));break;
    case ID_TYPE:  listPush(l, newStringValue(copyStr(getStrId((long) p->data)))); break;
    case STRING_TYPE:  listPush(l, newStringValue(copyStr((char*) p->data))); break;
    case STMTS_TYPE:
    case ASSIGN_TYPE:
    case ADD_TYPE:
    case SUB_TYPE:
    case MUL_TYPE:
    case DIV_TYPE:
    case IF_TYPE:
    case GT_TYPE:
    case LT_TYPE:
    case GE_TYPE:
    case LE_TYPE:
    case EQ_TYPE:
    case NE_TYPE:
    case AND_TYPE:
    case OR_TYPE:
    case NOT_TYPE:
    case WHILE_TYPE:
    case FUN_TYPE:
    case EXP_LIST_TYPE:
    case ID_LIST_TYPE:
    case CALL_TYPE:
    case TAIL_CALL_TYPE:
    case RETURN_TYPE:
    case BREAK_TYPE:
    case CONTINUE_TYPE:
    case MOD_TYPE:
    case LIST_TYPE:
    case LIST_ACCESS_TYPE:
    case FOR_TYPE:
    case ADDEQ_TYPE:
    case TIME_TYPE:
    case NONE_TYPE:
    case FOREACH_TYPE:
    case TRY_TYPE:
    case THROW_TYPE:
    case ADDADD_TYPE:
    case LOCAL_TYPE:
    case IMPORT_TYPE:
    case MODULE_ACCESS_TYPE: {
      int i, n = chldNum(p);
      for(i=0;i<n;i++)
        listPush(l, nodeToListValue(chld(p,i)));
      break;
    }
    default: error("unknown node type in nodeToListValue");
  }
  return res;
}

char* nodeTypeToString(NodeType type) {
  switch(type) {
    case INT_TYPE: return "int";
    case ID_TYPE: return "id";
    case STRING_TYPE: return "string";
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
    case ASSIGN_TYPE: return "=";
    case ADDEQ_TYPE:  return "+=";
    case TIME_TYPE: return "time";
    case RETURN_TYPE: return "return";
    case EXP_LIST_TYPE: return "exps";
    case ID_LIST_TYPE: return "ids";
    case STMTS_TYPE: return "stmts";
    case LIST_TYPE: return "list";
    case IF_TYPE: return "if";
    case NOT_TYPE: return "not";
    case WHILE_TYPE: return "while";
    case FUN_TYPE: return "fun";
    case CALL_TYPE: return "call";
    case TAIL_CALL_TYPE: return "tail_call";
    case BREAK_TYPE: return "break";
    case CONTINUE_TYPE: return "continue";
    case LIST_ACCESS_TYPE: return "list_access";
    case FOR_TYPE: return "for";
    case NONE_TYPE: return "none";
    case FOREACH_TYPE: return "foreach";
    case TRY_TYPE: return "try";
    case THROW_TYPE: return "throw";
    case ADDADD_TYPE: return "++";
    case LOCAL_TYPE: return "local";
    case IMPORT_TYPE: return "import";
    case MODULE_ACCESS_TYPE: return "module_access";
    default: error("unknown node type %d\n", type);
  }
  return 0; // never reach here
}

void freeNode(Node* t) {
  freeNodeC++;
  switch(t->type) {
    case INT_TYPE: 
    case ID_TYPE: break;
    case STRING_TYPE: tlFree(t->data); break;
    default: {
      int n = chldNum(t), i;        
      for(i=0; i<n; i++)
        freeNode(chld(t, i));
      freeList(t->data);
      break;
    }
  }
  tlFree(t);
}

void printAst(Node* ast) {
  printf("%s\n", valueToString(nodeToListValue(ast)));
}

