#include "util.h"
#include "list.h"

void* copy(void* t, int size) {
  void* res = malloc(size);
  memcpy(res, t, size);
  return res;
}

char* copyStr(char* t) {
  return copy(t, strlen(t) + 1);
}

char* catStr(char* s1, char* s2) {
  char* s = (char*) malloc(strlen(s1) + strlen(s2) + 1);
  s[0]=0;
  strcat(s, s1);
  strcat(s, s2);
  return s;
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

char* getFolder(char* s) {
  int i = strlen(s)-1;
  while(i && s[i]!='/')i--;
  if(i==0) return copyStr("./");
  else {
    char* res = copyStr(s);
    res[i+1]=0;
    return res;
  }
}

extern List* path;
FILE* openFromPath(char* s, char* mode) {
  int i, n = listSize(path);
  FILE* f = 0;
  for(i=0;i<n;i++) {
    char * p = listGet(path, i);
    char *fname = catStr(p, s);
    f = fopen(fname, mode);
    if(f)break;
  }
  return f;
}
