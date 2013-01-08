#include "tl.h"
#include "ast.h"
#include "util.h"
#include "list.h"

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

int chldNum(Node* t) {
  return listSize((List*) t->data);
}

Node* chld(Node* e, int i) {
  return listGet((List*) e->data, i);
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

char* nodeTypeToString(NodeType type) {
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
    case ASSIGN_TYPE: return "=";
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
    case LOCAL_TYPE: return "local";
    default: error("unknown node type %d\n", type);
  }
}

