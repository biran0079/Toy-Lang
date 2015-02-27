#include "tljmp.h"
#include "list.h"
#include "util.h"
#include "opStack.h"
#include <assert.h>

JmpMsg __jmpMsg__;

#ifndef USE_LEGACY_EVAL

static List* jumpBufs;

void initTljump() {
  jumpBufs = newList();
}

void cleanupTlJump() {
  assert(listSize(jumpBufs) == 0);
  freeList(jumpBufs);
}

int tlSetjmp() {
  BufBox* p = tlMalloc(sizeof(BufBox));
  listPush(jumpBufs, p);
  opStackSave();
  int res = setjmp(p->buf);
  if (res) {
    tlPopJumpBuf();
    opStackRestore();
  }
  return res;
}

void tlLongjmp(JmpMsgType type, Value *data) {
  __jmpMsg__.type = type;
  __jmpMsg__.data = data;
  tlPropagateJmp();
}

void tlPropagateJmp() {
  longjmp(listLast(jumpBufs), 999);
}

void tlPopJumpBuf() {
  tlFree(listPop(jumpBufs));
}

#else
void tlLongjmp(buf, JmpMsgType type, Value *data) {
  __jmpMsg__.type = type;
  __jmpMsg__.data = data;
  longjmp(buf, 999);
}
#endif
