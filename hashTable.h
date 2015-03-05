#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_

#include "core.h"
#include "list.h"

typedef struct LinkedList LinkedList;
struct LinkedList {
  void *key;
  void *value;
  LinkedList *next;
};

typedef unsigned int (*HashFunc)();
typedef int (*EqualsFunc)();

typedef struct HashTable {
  unsigned int cap, size;
  LinkedList **a;
  HashFunc h;
  EqualsFunc eq;
} HashTable;

HashTable *newHashTable(HashFunc h, EqualsFunc eq);
void freeHashTable(HashTable *t);
void *hashTablePut(HashTable *t, void *key, void *value);
void *hashTableGet(HashTable *t, void *key);
void *hashTableRemove(HashTable *t, void *key);
void hashTableClear(HashTable *t);
int hashTableSize(HashTable *t);

HashTable *newStringHashTable();
HashTable *newIntHashTable();
void freeStringHashTable(HashTable *t);
List *hashTableGetAllKeys(HashTable *t);
#endif
