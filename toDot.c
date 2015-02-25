#include "toDot.h"
#include "util.h"
#include "ast.h"

static char *maybeTruncateString(char *s) {
  int len = strlen(s);
  if (len > 10) {
    char *res = (char *)tlMalloc(14);
    memcpy(res, s, 10);
    res[10] = 0;
    strcat(res, "...");
    return res;
  }
  return copyStr(s);
}

static int dotNeedEscape(char c) {
  return c == '<' || c == '>' || c == '"' || c == '\n' || c == '|';
}

static char *escapeForDot(char *s) {
  int i;
  int ct = 0;
  for (i = 0; s[i]; i++)
    if (dotNeedEscape(s[i])) ct++;
  char *res = (char *)tlMalloc(strlen(s) + ct + 1);
  int j = 0;
  for (i = 0; s[i]; i++) {
    if (dotNeedEscape(s[i])) res[j++] = '\\';
    res[j++] = s[i];
  }
  res[j] = 0;
  return res;
}

static int nodeToDotInternal(FILE *o, Node *t, int *nextId);

static void nodeToDotHelper(FILE *o, Node *t, int *nextId, long n, ...) {
  int id = *nextId - 1;
  va_list l;
  char *s;
  int i;
  va_start(l, n);
  s = escapeForDot(nodeTypeToString(t->type));
  fprintf(o, "%d [label=\"{%s|{", id, s);
  tlFree(s);
  for (i = 0; i < n; i++) {
    if (i) fprintf(o, "|");
    fprintf(o, "<%d> %s", i, va_arg(l, char *));
  }
  fprintf(o, "}}\"];\n");
  for (i = 0; i < n; i++) {
    int to = nodeToDotInternal(o, chld(t, i), nextId);
    fprintf(o, "%d:%d -> %d;\n", id, i, to);
  }
  va_end(l);
}

static int nodeToDotInternal(FILE *o, Node *t, int *nextId) {
  // printf("%s\n", nodeTypeToString(t->type));
  int id = (*nextId)++;
  switch (t->type) {
    case INT_TYPE:
      fprintf(o, "%d [label=\"INT(%ld)\"];\n", id, (long)t->data);
      break;
    case ID_TYPE:
      fprintf(o, "%d [label=\"ID(%s)\"];\n", id, getStrId((long)t->data));
      break;
    case STRING_TYPE: {
      char *s = maybeTruncateString((char *)t->data);
      char *s2 = escapeForDot(s);
      fprintf(o, "%d [label=\"STRING(%s)\"];\n", id, s2);
      tlFree(s);
      tlFree(s2);
      break;
    }
    case NONE_TYPE:
      fprintf(o, "%d [label=\"none\"];\n", id);
      break;
    case BREAK_TYPE:
      fprintf(o, "%d [label=\"break\"]", id);
      break;
    case CONTINUE_TYPE:
      fprintf(o, "%d [label=\"continue\"]", id);
      break;

    case MODULE_ACCESS_TYPE:
    case EXP_LIST_TYPE:
    case ID_LIST_TYPE:
    case LIST_TYPE:
    case STMTS_TYPE: {
      fprintf(o, "%d [label=\"{%s|{", id, nodeTypeToString(t->type));
      int n = chldNum(t);
      int i;
      for (i = 0; i < n; i++) {
        if (i) fprintf(o, "|");
        fprintf(o, "<%d> %d", i, i);
      }
      fprintf(o, "}}\"];\n");
      for (i = 0; i < n; i++) {
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
      nodeToDotHelper(o, t, nextId, 2L, "op1", "op2");
      break;
    case TIME_TYPE:
    case THROW_TYPE:
    case RETURN_TYPE:
      nodeToDotHelper(o, t, nextId, 1L, "exp");
      break;
    case NOT_TYPE:
      nodeToDotHelper(o, t, nextId, 1L, "exp");
      break;
    case ADDADD_TYPE:
      nodeToDotHelper(o, t, nextId, 1L, "id");
      break;
    case LOCAL_TYPE:
      nodeToDotHelper(o, t, nextId, 1L, "ids");
      break;
    case IMPORT_TYPE:
      nodeToDotHelper(o, t, nextId, 1L, "ids");
      break;
    case IF_TYPE: {
      int n = chldNum(t), to;
      if (n == 2) {
        nodeToDotHelper(o, t, nextId, 2L, "cond", "then");
      } else {
        nodeToDotHelper(o, t, nextId, 3L, "cond", "then", "else");
      }
      break;
    }
    case TRY_TYPE: {
      int n = chldNum(t), to;
      if (n == 3) {
        nodeToDotHelper(o, t, nextId, 3L, "try", "excep id", "catch");
      } else {
        nodeToDotHelper(o, t, nextId, 4L, "try", "excep id", "catch",
                        "finally");
      }
      break;
    }
    case WHILE_TYPE:
      nodeToDotHelper(o, t, nextId, 2L, "cond", "do");
      break;
    case FUN_TYPE:
      nodeToDotHelper(o, t, nextId, 3L, "name", "args", "body");
      break;
    case CALL_TYPE:
    case TAIL_CALL_TYPE:
      nodeToDotHelper(o, t, nextId, 2L, "name", "args");
      break;
    case LIST_ACCESS_TYPE:
      nodeToDotHelper(o, t, nextId, 2L, "list", "index");
      break;
    case FOR_TYPE:
      nodeToDotHelper(o, t, nextId, 4L, "init", "cond", "incr", "body");
      break;
    case FOREACH_TYPE:
      nodeToDotHelper(o, t, nextId, 3L, "id", "list", "body");
      break;
    default:
      error("unknown type %d in valueToStringInternal\n", t->type);
  }
  return id;
}

void nodeToDot(FILE *o, Node *t) {
  static int id = 0;
  fprintf(o, "digraph G {\nnode [shape = record];\n");
  nodeToDotInternal(o, t, &id);
  fprintf(o, "}\n");
  id = 0;
}
