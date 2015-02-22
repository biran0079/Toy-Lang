#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_

#include "core.h"
#include "list.h"

typedef struct LinkedList LinkedList;
struct LinkedList {
  void *key;
  void* value;
  LinkedList* next;
};

typedef unsigned int(*HashFunc)();
typedef int(*EqualsFunc)();

typedef struct HashTable {
  unsigned int cap, size;
  LinkedList** a;
  HashFunc h;
  EqualsFunc eq;
} HashTable;

HashTable* newStringHashTable();
HashTable* newIntHashTable();
HashTable* newHashTable(HashFunc h, EqualsFunc eq);
void freeHashTable(HashTable* t);
void hashTablePut(HashTable* t, void* key, void* value);
void* hashTableGet(HashTable* t, void* key);
void* hashTableRemove(HashTable* t, void* key);
HashTable* hashTableCopy(HashTable* t);
void hashTableClear(HashTable* t);
void hashTableApplyAllValue(HashTable* t, ValueFunc f);
void hashTableAddAllToList(HashTable* t, List* q);
void freeStringHashTable(HashTable* t);
#endif
