#include "exception.h"
#include "util.h"

Exception* newException(ExecUnit* finally) {
  Exception* res = MALLOC(Exception);
  res->finally = finally;
  return res;
}

Exception* freeException(Exception* e) {
  freeExecUnit(e->finally);
  free(e);
}
