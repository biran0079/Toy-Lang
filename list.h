#ifndef _LIST_H_
#define _LIST_H_
#define listSize(l) (((List *)(l))->size)
#define listGet(l, i) (((List *)(l))->arr[i])

typedef struct List {
  void **arr;
  int size, cap;
} List;

List *newList();
void freeList(List *lst);
void listPushFront(List *lst, void *e);
void listPush(List *lst, void *e);
void *listPop(List *lst);
void *listLast(List *lst);
void *listSet(List *lst, int i, void *v);
List *listCopy(List *lst);
void listClear(List *lst);
void listPopTo(List *lst, int size);
void listSwap(List *lst, int i, int j);

typedef int (*Comparator)();
void listSort(List *lst, Comparator cmp);

void increaseSizeTo(List *lst, int size);

#endif
