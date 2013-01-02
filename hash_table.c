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
  res->cap = 89;
  res->size = 0;
  res->a = (LinkedList**) malloc(res->cap * sizeof(LinkedList*));
  memset(res->a, 0, res->cap * sizeof(LinkedList*));
  return res;
}

void hashTablePut(HashTable* t, char* key, void* value) {
  if(LOG){
    printf("put %s\n",key);
  }
  unsigned int idx = hash(key) % t->cap;
  LinkedList* l = MALLOC(LinkedList);
  l->key = key;
  l->value = value;
  l->next = t->a[idx];
  t->a[idx] = l;
  t->size++;
}

void* hashTableGet(HashTable* t, char* key){
  if(LOG){
    printf("get %s\n",key);
  }
  unsigned idx = hash(key) % t->cap;
  LinkedList* l = t->a[idx];
  while(l){
    if(strcmp(l->key, key)==0) return l->value;
    l = l->next;
  }
  return 0;
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

