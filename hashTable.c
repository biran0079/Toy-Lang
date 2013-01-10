#include "hashTable.h"
#include "list.h"
#include "util.h"

#define LOG 0

static unsigned int hash(char *s) {
  int h=0;
  while(*s){
    h*=131;
    h+=*s;
    s++;
  }
  return h;
}

static int TABLE_CAPACITY = 11;

extern int newHashTableC, freeHashTableC;

HashTable* newHashTable() {
  newHashTableC++;
  HashTable* res = MALLOC(HashTable);
  res->cap = TABLE_CAPACITY;
  res->size = 0;
  res->a = (LinkedList**) malloc(res->cap * sizeof(LinkedList*));
  memset(res->a, 0, res->cap * sizeof(LinkedList*));
  return res;
}

void freeHashTable(HashTable* t) {
  freeHashTableC++;
  int i;
  for(i = 0;i < t->cap; i++){
    LinkedList* l = t->a[i];
    while(l){
      LinkedList* nl = l->next;
      l->key = 0;
      l->value = 0;
      free(l);
      l = nl;
    }
  }
  free(t->a);
  t->a = 0;
  t->cap = t->size = 0;
  free(t);
}

static LinkedList* hashTableGetInternal(HashTable* t, char* key) {
  unsigned idx = hash(key) % t->cap;
  LinkedList* l = t->a[idx];
  while(l){
    if(strcmp(l->key, key)==0) return l;
    l = l->next;
  }
  return 0;
}

void hashTablePut(HashTable* t, char* key, void* value) {
  if(LOG){
    printf("put %s\n",key);
  }
  LinkedList* l = hashTableGetInternal(t, key);
  if(l) {
    l->value = value;
  } else {
    unsigned int idx = hash(key) % t->cap;
    l = MALLOC(LinkedList);
    l->key = key;
    l->value = value;
    l->next = t->a[idx];
    t->a[idx] = l;
    t->size++;
  }
}

void* hashTableGet(HashTable* t, char* key){
  if(LOG){
    printf("get %s\n",key);
  }
  LinkedList* l = hashTableGetInternal(t, key);
  return l ? l->value : 0;
}

HashTable* hashTableCopy(HashTable* t) {
  HashTable* res = newHashTable();
  int i;
  for(i=0;i<t->cap;i++){
    LinkedList* l = t->a[i];
    while(l){
      hashTablePut(res, l->key, l->value);
      l = l->next;
    }
  }
  return res;
}

void hashTableClear(HashTable* t) {
  int i;
  for(i=0;i<t->cap;i++) {
    LinkedList *l = t->a[i], *nl;
    while(l){
      nl = l->next;
      free(l);
      l = nl;
    }
    t->a[i]=0;
  }
  t->size = 0;
}

void hashTableApplyAllValue(HashTable* t, ValueFunc f) {
  int i;
  for(i=0;i<t->cap;i++) {
    LinkedList *l = t->a[i], *nl;
    while(l){
      nl = l->next;
      f(l->value);
      l = nl;
    }
  }
}

void hashTableAddAllToList(HashTable* t, List* q) {
  int i;
  for(i=0;i<t->cap;i++) {
    LinkedList *l = t->a[i], *nl;
    while(l){
      nl = l->next;
      listPush(q, l->value);
      l = nl;
    }
  }
}