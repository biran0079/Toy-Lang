#include "tl.h"

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
    case ADDEQ_TYPE: nodeToDotHelper(o, t, nextId, 2, "op1", "op2");break;
    case TIME_TYPE:
    case STR_TYPE:
    case ORD_TYPE:
    case LEN_TYPE:
    case THROW_TYPE: 
    case RETURN_TYPE: nodeToDotHelper(o, t, nextId, 1, "exp"); break;
    case NOT_TYPE: nodeToDotHelper(o, t, nextId, 1, "exp"); break;
    case ADDADD_TYPE: nodeToDotHelper(o, t, nextId, 1, "id"); break;
    case PRINT_TYPE: 
    case LOCAL_TYPE: nodeToDotHelper(o, t, nextId, 1, "exps"); break;
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

