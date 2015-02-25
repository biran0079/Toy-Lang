#include "idMap.h"
#include "hashTable.h"
#include "util.h"

static HashTable *idToIntMap, *intToIdMap;

void initIdMap() {
  idToIntMap = newStringHashTable();
  intToIdMap = newIntHashTable();
}

void cleanupIdMap() {
  freeStringHashTable(idToIntMap);
  freeHashTable(intToIdMap);
}

long getIntId(char *s) {
  long res = (long) hashTableGet(idToIntMap, s);
  if(!res){
    void* key = copyStr(s);
    void* value = (void*)(idToIntMap->size + 1L);
    hashTablePut(idToIntMap, key, value);
    hashTablePut(intToIdMap, value, key);
    res = idToIntMap->size;
  }
  return res;
}

char* getStrId(long id) {
  return hashTableGet(intToIdMap, (void*) id);
}
