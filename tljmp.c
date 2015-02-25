#include "tljmp.h"
#include <stdio.h>

extern JmpMsg __jmpMsg__;

void tlLongjmp(jmp_buf buf, JmpMsgType type, Value *data) {
  __jmpMsg__.type = type;
  __jmpMsg__.data = data;
  longjmp(buf, 999);
}
