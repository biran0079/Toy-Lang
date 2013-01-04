#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_
typedef struct LinkedList LinkedList;
struct LinkedList{
  char *key;
  void* value;
  LinkedList* next;
};

typedef struct HashTable {
  unsigned int cap, size;
  LinkedList** a;
} HashTable;

HashTable* newHashTable();
void hashTablePut(HashTable* t, char* key, void* value);
void* hashTableGet(HashTable* t, char* key);
HashTable* hashTableCopy(HashTable* t);
#endif
