#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include "execUnit.h"
#include <setjmp.h>
typedef struct Exception {
  jmp_buf buf;
  ExecUnit* finally;
} Exception;

Exception* newException(ExecUnit* finally);
void freeException(Exception* e);

#endif
