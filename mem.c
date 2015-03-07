#include "mem.h"
#include "util.h"
#include "closure.h"
#include "env.h"
#include "hashTable.h"
#include "opStack.h"
#include "eval.h"
#include "gc.h"
#include <assert.h>
#define MAX_BLOCK_SIZE 4 * 1024 * 1024

static ValuesBlock* head;
static int initSize = 1024;
extern int memoryUsage, memoryLimit;

static ValuesBlock* newValuesBlock(int size) {
  if (size > MAX_BLOCK_SIZE) {
    size = MAX_BLOCK_SIZE;
  }
#ifdef DEBUG_GC
  fprintf(stderr, "alloc %d\n", size);
#endif
  ValuesBlock* res = MALLOC(ValuesBlock);
  res->a = tlMalloc(size * sizeof(Value));
  res->capacity = size;
  res->top = 0;
  res->next = 0;
  return res;
}

static void freeValuesBlock(ValuesBlock* b) {
#ifdef DEBUG_GC
  fprintf(stderr, "free %d\n", b->capacity);
#endif
  tlFree(b->a);
  tlFree(b);
}

void initValuesBlock() { head = newValuesBlock(initSize); }

void cleanupValuesBlock() {
  ValuesBlock* cur = head;
  while (cur) {
    ValuesBlock* next = cur->next;
    freeValuesBlock(cur);
    cur = next;
  }
}

Value* allocValue() {
  gc();
  ValuesBlock* cur = head;
  while (1) {
    if (cur->top < cur->capacity) {
      Value* res = cur->a + cur->top;
      cur->top++;
      return res;
    }
    if (!cur->next) {
      cur->next = newValuesBlock(cur->capacity * 2);
    }
    cur = cur->next;
  }
}

typedef struct Iterator {
  ValuesBlock* b;
  int i;
} Iterator;

static Iterator* newIterator(ValuesBlock* b, int i) {
  Iterator* res = MALLOC(Iterator);
  res->b = b;
  res->i = i;
  return res;
}

static void freeIterator(Iterator* b) { tlFree(b); }

static Value* readNext(Iterator* it, Mark mark) {
  while (1) {
    if (it->i == it->b->top) {
      if (it->b->next) {
        assert(it->b->capacity == it->b->top);
        it->b = it->b->next;
        it->i = 0;
      } else {
        return 0;
      }
    } else {
      if (it->b->a[it->i].mark & mark) {
        return it->b->a + (it->i)++;
      } else {
        (it->i)++;
      }
    }
  }
}

static void writeAndUnmark(Iterator* it, Value* v, HashTable* addrMap) {
  if (v->mark == MARKED) v->mark = UNMARKED;
  if (it->i == it->b->top) {
    assert(it->b->next);
    assert(it->b->capacity == it->b->top);
    it->b = it->b->next;
    it->i = 0;
  }
  assert(it->i < it->b->top);
  hashTablePut(addrMap, v, it->b->a + it->i);
#ifdef DEBUG_GC
  printf("mapping %p -> %p\n", v, it->b->a + it->i);
#endif
  it->b->a[(it->i)++] = *v;
}

static void freeAllBlocksAfter(Iterator* it) {
  ValuesBlock* b = it->b;
  b->top = it->i;
  ValuesBlock* next = b->next;
  b->next = 0;
  ValuesBlock* cur = next;
  while (cur) {
    next = cur->next;
    freeValuesBlock(cur);
    cur = next;
  }
}

static void updateValuePointers(HashTable* addrMap) {
  opStackUpdateAddr(addrMap);

  Iterator* from = newIterator(head, 0);
  Value* v;
  Value* oldAddr, *newAddr;
  while ((v = readNext(from, -1))) {
#ifdef DEBUG_GC
    printf("updating %s @ %p\n", valueToString(v), v);
#endif
    switch (v->type) {
      case CLOSURE_VALUE_TYPE: {
        Closure* c = v->data;
        newAddr = hashTableGet(addrMap, c->e);
        c->e = newAddr;
        break;
      }
      case ENV_VALUE_TYPE: {
        Env* e = getEnvFromValue(v);
        List* keys = envGetAllIds(e);
        int i, n = listSize(keys);
        for (i = 0; i < n; i++) {
          long key = (long)listGet(keys, i);
          oldAddr = envGetLocal(e, key);
          newAddr = hashTableGet(addrMap, oldAddr);
          assert(newAddr);
          envPutLocal(e, key, newAddr);
        }
        freeList(keys);
        newAddr = hashTableGet(addrMap, e->parent);
        assert(newAddr);
        assert(newAddr->type == ENV_VALUE_TYPE ||
               newAddr->type == NONE_VALUE_TYPE);
        e->parent = newAddr;

        newAddr = hashTableGet(addrMap, e->envValue);
        assert(newAddr);
        assert(newAddr->type == ENV_VALUE_TYPE ||
               newAddr->type == NONE_VALUE_TYPE);
        e->envValue = newAddr;
        break;
      }
      case LIST_VALUE_TYPE: {
        List* l = v->data;
        int i, n = listSize(l);
        for (i = 0; i < n; i++) {
          oldAddr = listGet(l, i);
          newAddr = hashTableGet(addrMap, oldAddr);
          assert(newAddr);
          listSet(l, i, newAddr);
        }
        break;
      }
      case STRING_VALUE_TYPE:
      case INT_VALUE_TYPE:
      case BUILTIN_FUN_VALUE_TYPE:
      case NONE_VALUE_TYPE:
        break;
      default:
        error("cannot mark unknown value %d\n", v->type);
    }
  }
  freeIterator(from);
}

void consolidateMarkedValues() {
  Iterator* from = newIterator(head, 0);
  Iterator* to = newIterator(head, 0);
  HashTable* addrMap = newIntHashTable();
  Value* v;
  while ((v = readNext(from, MARKED))) {
    writeAndUnmark(to, v, addrMap);
  }
  freeAllBlocksAfter(to);
  updateValuePointers(addrMap);
  freeHashTable(addrMap);
  freeIterator(from);
  freeIterator(to);
}

void freeUnmarkedValues() {
  Iterator* it = newIterator(head, 0);
  Value* v;
  while ((v = readNext(it, UNMARKED))) {
    freeValue(v);
  }
  freeIterator(it);
}

void dumpValueMemory() {
  Iterator* it = newIterator(head, 0);
  printf("Value memory:\n");
  Value* v;
  while ((v = readNext(it, -1))) {
    printf("    %s @ %p\n", valueToString(v), v);
  }
  printf("\n");
}
