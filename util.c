#include "util.h"
#include "list.h"

int memoryUsage = 0;

typedef struct MemBlock { int size; } MemBlock;

void *tlMalloc(int size) {
  memoryUsage += size;
  MemBlock *m = malloc(size + sizeof(MemBlock));
  m->size = size;
  return m + 1;
}

void *tlRealloc(void *t, int size) {
  MemBlock *m = (MemBlock *)t - 1;
  memoryUsage += size - m->size;
  m = (MemBlock *)realloc(m, size + sizeof(MemBlock));
  m->size = size;
  return m + 1;
}

void tlFree(void *t) {
  MemBlock *m = (MemBlock *)t - 1;
  memoryUsage -= m->size;
  free(m);
}

void *copy(void *t, int size) {
  void *res = tlMalloc(size);
  memcpy(res, t, size);
  return res;
}

char *copyStr(char *t) { return copy(t, strlen(t) + 1); }

char *catStr(char *s1, char *s2) {
  char *s = (char *)tlMalloc(strlen(s1) + strlen(s2) + 1);
  s[0] = 0;
  strcat(s, s1);
  strcat(s, s2);
  return s;
}

long strToLong(char *s) {
  long res = 0;
  while (*s) {
    res *= 10;
    res += *s - '0';
    s++;
  }
  return res;
}

char *literalStringToString(char *s) {
  char *ss = tlMalloc(strlen(s) + 1);
  int l = 0;
  s++;  // skip "
  while (*s != '"') {
    if (*s == '\\') {
      s++;
      switch (*s) {
        case 'n':
          ss[l++] = '\n';
          break;
        case 'r':
          ss[l++] = '\r';
          break;
        case '"':
          ss[l++] = '"';
          break;
        case '\\':
          ss[l++] = '\\';
          break;
        default:
          ss[l++] = *s;
          break;
      }
    } else {
      ss[l++] = *s;
    }
    s++;
  }
  ss[l++] = 0;
  return tlRealloc(ss, l);
}

int error(char *format, ...) {
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
  exit(-1);
}

char *getFolder(char *s) {
  int i = strlen(s) - 1;
  while (i && s[i] != '/') i--;
  if (i == 0)
    return copyStr("./");
  else {
    char *res = copyStr(s);
    res[i + 1] = 0;
    return res;
  }
}

char *readFileWithPath(char *path) { return readFile(fopen(path, "r")); }

char *readFile(FILE *f) {
  if (!f) return 0;
  fseek(f, 0L, SEEK_END);
  int sz = ftell(f);
  fseek(f, 0L, SEEK_SET);
  char *res = (char *)tlMalloc(sz + 1);
  char *s = res;
  while (sz) {
    size_t len = fread(s, 1, sz, f);
    if (len == 0) break;
    s += len;
    sz -= len;
  }
  *s = 0;
  return res;
}
