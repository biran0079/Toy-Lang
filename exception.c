#include "exception.h"
#include "util.h"

Exception* newException(ExecUnit* finally) {
  Exception* res = MALLOC(Exception);
  res->finally = finally;
  return res;
}

void freeException(Exception* e) {
  if(e->finally) freeExecUnit(e->finally);
  tlFree(e);
}
