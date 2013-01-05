#ifndef _LIST_H_
#define _LIST_H_

typedef struct List {
  void** arr;
  int size, cap;
} List;

List* newList();
void listPush(List* lst, void* e);
void* listGet(List* lst, int idx);
void* listPop(List* lst); 
int listSize(List* lst);
void* listLast(List* lst);
void listSet(List* lst, int i, void* v);
List* listCopy(List* lst);
void listClear(List* lst);

#endif
