#ifndef __MEM_H__
#define __MEM_H__

#include "value.h"

extern List* allValues, *deadValues;

void initMem();
void cleanupMem();

Value* allocValue();
void freeUnmarkedValues();

int getInMemoryValueCount();

void dumpValueMemory();
#endif
