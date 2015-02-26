#ifndef __MEM_H__
#define __MEM_H__

#include "value.h"

typedef struct ValuesBlock ValuesBlock;
struct ValuesBlock {
  Value* a;
  int top;
  int capacity;
  ValuesBlock* next;
};

void initValuesBlock();
void cleanupValuesBlock();

Value* allocValue();
void consolidateMarkedValues();
void freeUnmarkedValues();

int getInMemoryValueCount();

void dumpValueMemory();
#endif
