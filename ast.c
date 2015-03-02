#include "value.h"
#include "eval.h"
#include "ast.h"
#include "util.h"
#include "list.h"
#include "opStack.h"

int newNodeC = 0, freeNodeC = 0;

Node *newNode(NodeType t, void *i) {
  newNodeC++;
  Node *res = MALLOC(Node);
  res->type = t;
  res->data = i;
  res->eval = 0;
  return res;
}

Node *newNode2(NodeType t, int n, ...) {
  newNodeC++;
  Node *res = MALLOC(Node);
  List *l = newList();
  va_list v;
  int i;
  va_start(v, n);
  for (i = 0; i < n; i++) {
    listPush(l, va_arg(v, Node *));
  }
  res->type = t;
  res->data = (void *)l;
  res->eval = 0;
  va_end(v);
  return res;
}

void markTailRecursions(Node *t) {
  switch (t->type) {
    case STMTS_TYPE: {
      int n = chldNum(t);
      if (n) markTailRecursions(chld(t, n - 1));
      break;
    }
    case IF_TYPE: {
      markTailRecursions(chld(t, 1));
      if (chldNum(t) > 2)  // has "else"
        markTailRecursions(chld(t, 2));
      break;
    }
    case RETURN_TYPE: {
      if (chldNum(t)) markTailRecursions(chld(t, 0));
      break;
    }
    case CALL_TYPE: {
      t->type = TAIL_CALL_TYPE;
      break;
    }
    default:
      break;
  }
}

static int nodeToStringInternal(Node *p, char *s, int n) {
  Value *res = newListValue(newList());
  int len = 0;
  len += mySnprintf(s + len, n - len, " (%s", nodeTypeToString(p->type));
  switch (p->type) {
    case INT_TYPE:
      len += mySnprintf(s + len, n - len, " %d", (long)p->data);
      break;
    case ID_TYPE:
      len += mySnprintf(s + len, n - len, " %s", getStrId((long)p->data));
      break;
    case STRING_TYPE:
      len += mySnprintf(s + len, n - len, " %s", (char *)p->data);
      break;
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
      int i, chldN = chldNum(p);
      for (i = 0; i < chldN; i++) {
        len += nodeToStringInternal(chld(p, i), s + len, n - len);
      }
      break;
    }
    default:
      error("unknown node type in nodeToListValue");
  }
  len += mySnprintf(s + len, n - len, ")");
  return len;
}

char* nodeToString(Node* p) {
  int l = nodeToStringInternal(p, 0, 0) + 1;
  char *s = (char *)tlMalloc(l);
  nodeToStringInternal(p, s, l);
  return s;
}

Value *nodeToListValue(Node *p) {
  Value *res = newListValue(newList());
  listValuePush(res, newStringValue(copyStr(nodeTypeToString(p->type))));
  switch (p->type) {
    case INT_TYPE:
      listValuePush(res, newIntValue((long)p->data));
      break;
    case ID_TYPE:
      listValuePush(res, newStringValue(copyStr(getStrId((long)p->data))));
      break;
    case STRING_TYPE:
      listValuePush(res, newStringValue(copyStr((char *)p->data)));
      break;
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
      for (i = 0; i < n; i++) listValuePush(res, nodeToListValue(chld(p, i)));
      break;
    }
    default:
      error("unknown node type in nodeToListValue");
  }
  return res;
}

char *nodeTypeToString(NodeType type) {
  switch (type) {
    case INT_TYPE:
      return "int";
    case ID_TYPE:
      return "id";
    case STRING_TYPE:
      return "string";
    case ADD_TYPE:
      return "+";
    case SUB_TYPE:
      return "-";
    case MUL_TYPE:
      return "*";
    case DIV_TYPE:
      return "/";
    case MOD_TYPE:
      return "%";
    case GT_TYPE:
      return ">";
    case LT_TYPE:
      return "<";
    case GE_TYPE:
      return ">=";
    case LE_TYPE:
      return "<=";
    case EQ_TYPE:
      return "==";
    case NE_TYPE:
      return "!=";
    case AND_TYPE:
      return "&&";
    case OR_TYPE:
      return "||";
    case ASSIGN_TYPE:
      return "=";
    case ADDEQ_TYPE:
      return "+=";
    case TIME_TYPE:
      return "time";
    case RETURN_TYPE:
      return "return";
    case EXP_LIST_TYPE:
      return "exps";
    case ID_LIST_TYPE:
      return "ids";
    case STMTS_TYPE:
      return "stmts";
    case LIST_TYPE:
      return "list";
    case IF_TYPE:
      return "if";
    case NOT_TYPE:
      return "not";
    case WHILE_TYPE:
      return "while";
    case FUN_TYPE:
      return "fun";
    case CALL_TYPE:
      return "call";
    case TAIL_CALL_TYPE:
      return "tail_call";
    case BREAK_TYPE:
      return "break";
    case CONTINUE_TYPE:
      return "continue";
    case LIST_ACCESS_TYPE:
      return "list_access";
    case FOR_TYPE:
      return "for";
    case NONE_TYPE:
      return "none";
    case FOREACH_TYPE:
      return "foreach";
    case TRY_TYPE:
      return "try";
    case THROW_TYPE:
      return "throw";
    case ADDADD_TYPE:
      return "++";
    case LOCAL_TYPE:
      return "local";
    case IMPORT_TYPE:
      return "import";
    case MODULE_ACCESS_TYPE:
      return "module_access";
    default:
      error("unknown node type %d\n", type);
  }
  return 0;  // never reach here
}

void freeNode(Node *t) {
  freeNodeC++;
  switch (t->type) {
    case INT_TYPE:
    case ID_TYPE:
      break;
    case STRING_TYPE:
      tlFree(t->data);
      break;
    default: {
      int n = chldNum(t), i;
      for (i = 0; i < n; i++) freeNode(chld(t, i));
      freeList(t->data);
      break;
    }
  }
  tlFree(t);
}

void printAst(Node *ast) {
  char *s = nodeToString(ast);
  printf("%s\n", s);
  tlFree(s);
}

long getIdFromNode(Node *node) {
  assert(node->type == ID_TYPE);
  return (long)node->data;
}

Node *postProcessAst(Node *p) {
  switch (p->type) {
    case STMTS_TYPE:
      p->eval = evalStmts;
      break;
    case EXP_LIST_TYPE:
      p->eval = evalExpList;
      break;
    case NONE_TYPE:
      p->eval = evalNone;
      break;
    case LIST_TYPE:
      p->eval = evalList;
      break;
    case LIST_ACCESS_TYPE:
      p->eval = evalListAccess;
      break;
    case TAIL_CALL_TYPE:
      p->eval = evalTailRecursion;
      break;
    case CALL_TYPE:
      p->eval = evalCall;
      break;
    case RETURN_TYPE:
      p->eval = evalReturn;
      break;
    case ID_TYPE:
      p->eval = evalId;
      break;
    case INT_TYPE:
      p->eval = evalInt;
      break;
    case STRING_TYPE:
      p->eval = evalString;
      break;
    case ASSIGN_TYPE:
      p->eval = evalAssign;
      break;
    case ADDEQ_TYPE:
      p->eval = evalAddEq;
      break;
    case ADD_TYPE:
      p->eval = evalAdd;
      break;
    case SUB_TYPE:
      p->eval = evalSub;
      break;
    case MUL_TYPE:
      p->eval = evalMul;
      break;
    case DIV_TYPE:
      p->eval = evalDiv;
      break;
    case MOD_TYPE:
      p->eval = evalMod;
      break;
    case IF_TYPE:
      p->eval = evalIf;
      break;
    case FOR_TYPE:
      p->eval = evalFor;
      break;
    case FOREACH_TYPE:
      p->eval = evalForEach;
      break;
    case WHILE_TYPE:
      p->eval = evalWhile;
      break;
    case CONTINUE_TYPE:
      p->eval = evalContinue;
      break;
    case BREAK_TYPE:
      p->eval = evalBreak;
      break;
    case GT_TYPE:
      p->eval = evalGT;
      break;
    case LT_TYPE:
      p->eval = evalLT;
      break;
    case GE_TYPE:
      p->eval = evalGE;
      break;
    case LE_TYPE:
      p->eval = evalLE;
      break;
    case EQ_TYPE:
      p->eval = evalEQ;
      break;
    case NE_TYPE:
      p->eval = evalNE;
      break;
    case AND_TYPE:
      p->eval = evalAnd;
      break;
    case OR_TYPE:
      p->eval = evalOr;
      break;
    case NOT_TYPE:
      p->eval = evalNot;
      break;
    case FUN_TYPE:
      p->eval = evalFun;
      break;
    case TIME_TYPE:
      p->eval = evalTime;
      break;
    case TRY_TYPE:
      p->eval = evalTry;
      break;
    case THROW_TYPE:
      p->eval = evalThrow;
      break;
    case ADDADD_TYPE:
      p->eval = evalAddAdd;
      break;
    case LOCAL_TYPE:
      p->eval = evalLocal;
      break;
    case IMPORT_TYPE:
      p->eval = evalImport;
      break;
    case MODULE_ACCESS_TYPE:
      p->eval = evalModuleAccess;
      break;
    default:
      p->eval = evalError;
      break;  // types should never eval id list
  }
  switch (p->type) {
    case INT_TYPE:
    case STRING_TYPE:
    case ID_TYPE:
      break;  // leaves
    default: {
      int n = chldNum(p), i;
      for (i = 0; i < n; i++) postProcessAst(chld(p, i));
    }
  }
  return p;
}
