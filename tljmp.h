#ifndef _TLJMP_H_
#define _TLJMP_H_

#include <setjmp.h>

typedef enum JmpMsgType {
  BREAK_MSG_TYPE,
  CONTINUE_MSG_TYPE,
  RETURN_MSG_TYPE,
  TAIL_CALL_MSG_TYPE,
  EXCEPTION_MSG_TYPE,
} JmpMsgType;

typedef struct JmpMsg {
  JmpMsgType type;
  void* data;
} JmpMsg;


void tlLongjmp(jmp_buf buf, JmpMsgType type, void* data);

#endif
