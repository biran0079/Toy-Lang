#include "util.h"

void* copy(void* t, int size) {
  void* res = malloc(size);
  memcpy(res, t, size);
  return res;
}

char* copyStr(char* t) {
  return copy(t, strlen(t) + 1);
}

long strToLong(char* s){
  long res = 0;
  while(*s) {
    res*=10;
    res+=*s-'0';
    s++;
  }
  return res;
}

char* literalStringToString(char *s) {
  char *ss = malloc(strlen(s)+1);
  int l = 0;
  s++;  // skil "
  while(*s!='"') {
    if(*s == '\\') {
      s++;
      switch(*s){
        case 'n' : ss[l++] = '\n'; break;
        case 'r' : ss[l++] = '\r'; break;
        case '"' : ss[l++] = '"'; break;
        case '\\' : ss[l++] = '\\'; break;
        default:  ss[l++] = *s; break;
      }
    }else{
      ss[l++] = *s;
    }
    s++;
  }
  ss[l++] = 0;
  return realloc(ss, l);
}

void error(char* format,...) {
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
  exit(-1);
}
