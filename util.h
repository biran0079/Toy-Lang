#ifndef _UTIL_H_
#define _UTIL_H_

#define MALLOC(T) (T*)malloc(sizeof(T))
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <setjmp.h>
#include <time.h>
#include <stdarg.h>

void* copy(void* t, int size);
long strToLong(char* s);
char* literalStringToString(char *s);
void error(char* msg, ...);

#endif