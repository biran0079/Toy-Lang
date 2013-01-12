#ifndef _GC_H_
#define _GC_H_

typedef struct GCRecord {
  int before, after;
} GCRecord;

void clearGCHistory();
void forceGC();
void gc();

#endif
