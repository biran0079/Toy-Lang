#ifndef _TLJMP_H_
#define _TLJMP_H_

#include "core.h"
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
  Value *data;
} JmpMsg;

extern JmpMsg __jmpMsg__;

#ifndef USE_LEGACY_EVAL
typedef struct BufBox {
  jmp_buf buf;
} BufBox;

void initTljump();
void cleanupTlJump();
int tlSetjmp();
void tlLongjmp(JmpMsgType type, Value *data);
void tlPropagateJmp();
void tlPopJumpBuf();
#else
void tlLongjmp(jmp_buf buf, JmpMsgType type, Value *data);
#endif

#endif
