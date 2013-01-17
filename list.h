#ifndef _LIST_H_
#define _LIST_H_
#define listSize(l) (((List*)(l))->size)
#define listGet(l,i) (((List*)(l))->arr[i])

typedef struct List {
  void** arr;
  int size, cap;
} List;

List* newList();
void freeList(List* lst);
void listPush(List* lst, void* e);
void* listPop(List* lst); 
void* listLast(List* lst);
void listSet(List* lst, int i, void* v);
List* listCopy(List* lst);
void listClear(List* lst);

typedef int (*Comparator)();
void listSort(List* lst, Comparator cmp);

#endif
