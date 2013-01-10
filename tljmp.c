#include "tljmp.h"
#include <stdio.h>

JmpMsg __jmpMsg__;

void tlLongjmp(jmp_buf buf, JmpMsgType type, void* data) {
  __jmpMsg__.type = type;
  __jmpMsg__.data = data;
  longjmp(buf, 999);
}
