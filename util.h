#ifndef _UTIL_H_
#define _UTIL_H_

#define MALLOC(T) (T *) tlMalloc(sizeof(T))
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <setjmp.h>
#include <time.h>
#include <stdarg.h>

void *tlMalloc(int size);
void *tlRealloc(void *t, int size);
void tlFree(void *t);
void *copy(void *t, int size);
char *copyStr(char *t);
char *catStr(char *s1, char *s2);
long strToLong(char *s);
char *literalStringToString(char *s);
int error(char *msg, ...);
char *getFolder(char *s);
FILE *openFromPath(char *s, char *mode);
char *readFileWithPath(char *path);
char *readFile(FILE *f);

#endif
