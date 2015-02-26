#include "hashTable.h"
#include "list.h"
#include "util.h"

int newHashTableC = 0, freeHashTableC = 0;

static unsigned int stringHash(unsigned char *s) {
  unsigned int h = 0;
  while (*s) {
    h *= 127;
    h += *s;
    s++;
  }
  return h;
}

static int stringEqual(char *s1, char *s2) { return strcmp(s1, s2) == 0; }

static unsigned int intHash(long n) { return (unsigned int)n; }

static int intEqual(long a, long b) { return a == b; }

static int INIT_CAPACITY = 8;

extern int newHashTableC, freeHashTableC;

HashTable *newStringHashTable() {
  return newHashTable(stringHash, stringEqual);
}

HashTable *newIntHashTable() { return newHashTable(intHash, intEqual); }

HashTable *newHashTable(HashFunc h, EqualsFunc eq) {
  newHashTableC++;
  HashTable *res = MALLOC(HashTable);
  res->cap = INIT_CAPACITY;
  res->size = 0;
  res->a = (LinkedList **)tlMalloc(res->cap * sizeof(LinkedList *));
  res->h = h;
  res->eq = eq;
  memset(res->a, 0, res->cap * sizeof(LinkedList *));
  return res;
}

static void rehash(HashTable *t) {
  LinkedList **old = t->a;
  int oldCap = t->cap;
  t->cap *= 2;
  t->a = (LinkedList **)tlMalloc(t->cap * sizeof(LinkedList *));
  t->size = 0;
  memset(t->a, 0, t->cap * sizeof(LinkedList *));
  int i;
  for (i = 0; i < oldCap; i++) {
    LinkedList *l = old[i], *next;
    while (l) {
      hashTablePut(t, l->key, l->value);
      next = l->next;
      l->next = 0;
      tlFree(l);
      l = next;
    }
  }
  tlFree(old);
}

void freeHashTable(HashTable *t) {
  freeHashTableC++;
  int i;
  for (i = 0; i < t->cap; i++) {
    LinkedList *l = t->a[i];
    while (l) {
      LinkedList *nl = l->next;
      l->key = 0;
      l->value = 0;
      l->next = 0;
      tlFree(l);
      l = nl;
    }
  }
  tlFree(t->a);
  t->a = 0;
  t->cap = t->size = 0;
  tlFree(t);
}

static LinkedList *hashTableGetInternal(HashTable *t, void *key) {
  unsigned idx = t->h(key) % t->cap;
  LinkedList *l = t->a[idx];
  while (l) {
    if (t->eq(l->key, key)) return l;
    l = l->next;
  }
  return 0;
}

void *hashTableRemove(HashTable *t, void *key) {
  unsigned idx = t->h(key) % t->cap;
  LinkedList *l = t->a[idx], *p = 0;
  while (l) {
    if (t->eq(l->key, key)) break;
    p = l;
    l = l->next;
  }
  if (l == 0) return 0;
  void *res = l->value;
  if (!p) {
    t->a[idx] = l->next;
    tlFree(l);
  } else {
    p->next = l->next;
    tlFree(l);
  }
  return res;
}

void hashTablePut(HashTable *t, void *key, void *value) {
  if (t->size > t->cap) rehash(t);
  LinkedList *l = hashTableGetInternal(t, key);
  if (l) {
    l->value = value;
  } else {
    unsigned int idx = t->h(key) % t->cap;
    l = MALLOC(LinkedList);
    l->key = key;
    l->value = value;
    l->next = t->a[idx];
    t->a[idx] = l;
    t->size++;
  }
}

void *hashTableGet(HashTable *t, void *key) {
  LinkedList *l = hashTableGetInternal(t, key);
  return l ? l->value : 0;
}

HashTable *hashTableCopy(HashTable *t) {
  HashTable *res = newHashTable(t->h, t->eq);
  int i;
  for (i = 0; i < t->cap; i++) {
    LinkedList *l = t->a[i];
    while (l) {
      hashTablePut(res, l->key, l->value);
      l = l->next;
    }
  }
  return res;
}

void hashTableClear(HashTable *t) {
  int i;
  for (i = 0; i < t->cap; i++) {
    LinkedList *l = t->a[i], *nl;
    while (l) {
      nl = l->next;
      tlFree(l);
      l = nl;
    }
    t->a[i] = 0;
  }
  t->size = 0;
}

void hashTableApplyAllValue(HashTable *t, ValueFunc f) {
  int i;
  for (i = 0; i < t->cap; i++) {
    LinkedList *l = t->a[i], *nl;
    while (l) {
      nl = l->next;
      f(l->value);
      l = nl;
    }
  }
}

void hashTableAddAllToList(HashTable *t, List *q) {
  int i;
  for (i = 0; i < t->cap; i++) {
    LinkedList *l = t->a[i], *nl;
    while (l) {
      nl = l->next;
      listPush(q, l->value);
      l = nl;
    }
  }
}

void freeStringHashTable(HashTable *t) {
  int i;
  for (i = 0; i < t->cap; i++) {
    LinkedList *l = t->a[i];
    while (l) {
      tlFree(l->key);
      l = l->next;
    }
  }
  freeHashTable(t);
}
