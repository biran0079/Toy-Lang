#include "tl.h"
#include "hash_table.h"

#define LOG 0

static unsigned int hash(char *s) {
  int h=0;
  while(*s){
    h*=132;
    h+=*s;
    s++;
  }
  return h;
}

HashTable* newHashTable() {
  HashTable* res = MALLOC(HashTable);
  res->size = 89513;
  res->a = (LinkedList**) malloc(res->size * sizeof(LinkedList*));
  memset(res->a, 0, res->size * sizeof(LinkedList*));
  return res;
}

void hashTablePut(HashTable* t, char* key, void* value) {
  if(LOG){
    printf("put %s\n",key);
  }
  unsigned int idx = hash(key) % t->size;
  LinkedList* l = MALLOC(LinkedList);
  l->key = key;
  l->value = value;
  l->next = t->a[idx];
  t->a[idx] = l;
}

void* hashTableGet(HashTable* t, char* key){
  if(LOG){
    printf("get %s\n",key);
  }
  unsigned idx = hash(key) % t->size;
  if(idx<0)idx+=t->size;
  LinkedList* l = t->a[idx];
  while(l){
    if(strcmp(l->key, key)==0) return l->value;
    l = l->next;
  }
  return 0;
}
