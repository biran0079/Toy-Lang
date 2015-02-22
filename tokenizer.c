#include "list.h"
#include "util.h"
#include "tokenizer.h"

char* tokenTypeToStr(Token_t type) {
  switch (type) {
    case INT_T: return "int";
    case STRING_T: return "string";
    case ID_T: return "id";
    case WHILE_T: return "while";
    case FOR_T: return "for";
    case FUN_T: return "fun";
    case RETURN_T: return "return";
    case LAMBDA_T: return "lambda";
    case NONE_T: return "none";
    case LOCAL_T: return "local";
    case TIME_T: return "time";
    case BREAK_T: return "break";
    case CONTINUE_T: return "continue";
    case TRY_T: return "try";
    case CATCH_T: return "catch";
    case FINALLY_T: return "finally";
    case THROW_T: return "throw";
    case IF_T: return "if";
    case ELSE_T: return "else";
    case ADDADD_T: return "addadd";
    case IMPORT_T: return "import";
    case ADD_T: return "add";
    case SUB_T: return "sub";
    case MUL_T: return "mul";
    case DIV_T: return "div";
    case MOD_T: return "mod";
    case DOT_T: return "dot";
    case AND_T: return "and";
    case OR_T: return "or";
    case COMMA_T: return "comma";
    case NOT_T: return "not";
    case EQ_T: return "eq";
    case NE_T: return "ne";
    case GT_T: return "gt";
    case LT_T: return "lt";
    case GE_T: return "ge";
    case LE_T: return "le";
    case ASSIGN_T: return "assign";
    case ADDEQ_T: return "addeq";
    case OP_B_T: return "op_b";
    case CLO_B_T: return "clo_b";
    case OP_CB_T: return "op_cb";
    case CLO_CB_T: return "clo_cb";
    case OP_SB_T: return "op_sb";
    case CLO_SB_T: return "clo_sb";
    case SEMICOLON_T: return "semicolon";
    case COLON_T: return "colon";
    default: return "unknown";
  }
}

Token* newToken(Token_t type, void* data) {
  Token* res = MALLOC(Token);
  res->type = type;
  res->data = data;
  return res;
}

int startsWithAndConsume(char** sp, char* p) {
  int len = strlen(p);
  if (strncmp(*sp, p, len) == 0) {
    *sp += len;
    return 1;
  }
  return 0;
}

void skipWhiteSpaces(char** sp) {
  while (1) {
    switch (**sp) {
      case ' ': case '\t': case '\r': case '\n': (*sp)++; break;
      case 0: return;
      default: return;
    }
  }
}

int isDigit(char c) {
  return c >= '0' && c <= '9';
}

int isLetter(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

long parseInt(char** sp) {
  long res = 0;
  char* s = *sp;
  do {
    res *= 10;
    res += *s - '0';
    s++;
  } while (isDigit(*s));
  *sp = s;
  return res;
}

char* parseString(char** sp) {
  char* s = *sp;
  char* res = tlMalloc(strlen(s) + 1);
  s++; // skip "
  int i = 0;
  while (*s != '"') {
    if (*s == '\\') {
      s++;
      switch (*s) {
        case 'n': res[i++] = '\n'; break;
        case 'r': res[i++] = '\r'; break;
        case '"': res[i++] = '\"'; break;
        case '\\': res[i++] = '\\'; break;
        default: res[i++] = *s; break;
      }
    } else {
      res[i++] = *s;
    }
    s++;
  }
  s++; // skip "
  res[i] = 0;
  *sp = s;
  return res;
}

char* parseID(char** sp) {
  char* s = *sp;
  int len = 0;
  while (isLetter(*s) || isDigit(*s)) {
    s++;
    len++;
  }
  s = *sp;
  char* res = (char*) tlMalloc(len + 1);
  int i;
  for (i = 0; i < len; i++)
    res[i] = s[i];
  res[len] = 0;
  *sp += len;
  return res;
}

List* tokenize(char* s) {
  List* res = newList();
  while (1) {
    char** sp = &s;
    skipWhiteSpaces(sp);
    if (0 == *s) break;
    if (isDigit(*s)) 
      listPush(res, newToken(INT_T, (void*) parseInt(sp)));
    else if (*s == '"')
      listPush(res, newToken(STRING_T, (void*) parseString(sp)));
    else if (startsWithAndConsume(sp, "if")) 
      listPush(res, newToken(IF_T, (void*) 0));
    else if (startsWithAndConsume(sp, "else")) 
      listPush(res, newToken(ELSE_T, (void*) 0));
    else if (startsWithAndConsume(sp, "while")) 
      listPush(res, newToken(WHILE_T, (void*) 0));
    else if (startsWithAndConsume(sp, "break")) 
      listPush(res, newToken(BREAK_T, (void*) 0));
    else if (startsWithAndConsume(sp, "continue")) 
      listPush(res, newToken(CONTINUE_T, (void*) 0));
    else if (startsWithAndConsume(sp, "fun")) 
      listPush(res, newToken(FUN_T, (void*) 0));
    else if (startsWithAndConsume(sp, "for")) 
      listPush(res, newToken(FOR_T, (void*) 0));
    else if (startsWithAndConsume(sp, "lambda")) 
      listPush(res, newToken(LAMBDA_T, (void*) 0));
    else if (startsWithAndConsume(sp, "return")) 
      listPush(res, newToken(RETURN_T, (void*) 0));
    else if (startsWithAndConsume(sp, "time")) 
      listPush(res, newToken(TIME_T, (void*) 0));
    else if (startsWithAndConsume(sp, "none")) 
      listPush(res, newToken(NONE_T, (void*) 0));
    else if (startsWithAndConsume(sp, "try")) 
      listPush(res, newToken(TRY_T, (void*) 0));
    else if (startsWithAndConsume(sp, "catch")) 
      listPush(res, newToken(CATCH_T, (void*) 0));
    else if (startsWithAndConsume(sp, "finally")) 
      listPush(res, newToken(FINALLY_T, (void*) 0));
    else if (startsWithAndConsume(sp, "throw")) 
      listPush(res, newToken(THROW_T, (void*) 0));
    else if (startsWithAndConsume(sp, "local")) 
      listPush(res, newToken(LOCAL_T, (void*) 0));
    else if (startsWithAndConsume(sp, "import")) 
      listPush(res, newToken(IMPORT_T, (void*) 0));
    else if (startsWithAndConsume(sp, "++")) 
      listPush(res, newToken(ADDADD_T, (void*) 0));
    else if (startsWithAndConsume(sp, "+=")) 
      listPush(res, newToken(ADDEQ_T, (void*) 0));
    else if (startsWithAndConsume(sp, ">=")) 
      listPush(res, newToken(GE_T, (void*) 0));
    else if (startsWithAndConsume(sp, "<=")) 
      listPush(res, newToken(LE_T, (void*) 0));
    else if (startsWithAndConsume(sp, ">")) 
      listPush(res, newToken(GT_T, (void*) 0));
    else if (startsWithAndConsume(sp, "<")) 
      listPush(res, newToken(LT_T, (void*) 0));
    else if (startsWithAndConsume(sp, "==")) 
      listPush(res, newToken(EQ_T, (void*) 0));
    else if (startsWithAndConsume(sp, "=")) 
      listPush(res, newToken(ASSIGN_T, (void*) 0));
    else if (startsWithAndConsume(sp, "!=")) 
      listPush(res, newToken(NE_T, (void*) 0));
    else if (startsWithAndConsume(sp, "!")) 
      listPush(res, newToken(NOT_T, (void*) 0));
    else if (startsWithAndConsume(sp, "&&")) 
      listPush(res, newToken(AND_T, (void*) 0));
    else if (startsWithAndConsume(sp, "||")) 
      listPush(res, newToken(OR_T, (void*) 0));
    else if (startsWithAndConsume(sp, ",")) 
      listPush(res, newToken(COMMA_T, (void*) 0));
    else if (startsWithAndConsume(sp, "+")) 
      listPush(res, newToken(ADD_T, (void*) 0));
    else if (startsWithAndConsume(sp, "-")) 
      listPush(res, newToken(SUB_T, (void*) 0));
    else if (startsWithAndConsume(sp, "*")) 
      listPush(res, newToken(MUL_T, (void*) 0));
    else if (startsWithAndConsume(sp, "/")) 
      listPush(res, newToken(DIV_T, (void*) 0));
    else if (startsWithAndConsume(sp, "%")) 
      listPush(res, newToken(MOD_T, (void*) 0));
    else if (startsWithAndConsume(sp, ".")) 
      listPush(res, newToken(DOT_T, (void*) 0));
    else if (startsWithAndConsume(sp, ";")) 
      listPush(res, newToken(SEMICOLON_T, (void*) 0));
    else if (startsWithAndConsume(sp, ":")) 
      listPush(res, newToken(COLON_T, (void*) 0));
    else if (startsWithAndConsume(sp, "(")) 
      listPush(res, newToken(OP_B_T, (void*) 0));
    else if (startsWithAndConsume(sp, ")")) 
      listPush(res, newToken(CLO_B_T, (void*) 0));
    else if (startsWithAndConsume(sp, "[")) 
      listPush(res, newToken(OP_SB_T, (void*) 0));
    else if (startsWithAndConsume(sp, "]")) 
      listPush(res, newToken(CLO_SB_T, (void*) 0));
    else if (startsWithAndConsume(sp, "{")) 
      listPush(res, newToken(OP_CB_T, (void*) 0));
    else if (startsWithAndConsume(sp, "}")) 
      listPush(res, newToken(CLO_CB_T, (void*) 0));
    else if (isLetter(*s)) 
      listPush(res, newToken(ID_T, (void*) parseID(sp)));
    else 
      error("fail to parse at %s\n", s);
  }
  return res;
}

#ifdef BUILD_TOKENIZER
int main(int argc, char** args) {
  init(argc, args);
  char* code = readFile(args[1]);
  List* tokens = tokenize(code);
  int i;
  for (i = 0; i < listSize(tokens); i++) {
    printf("%s\n", tokenTypeToStr(((Token*) listGet(tokens, i))->type));
  }
  return 0;
}
#endif
