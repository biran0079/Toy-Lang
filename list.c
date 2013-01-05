#include "list.h"
#include "tl.h"
#include<setjmp.h>

static void resize(List* lst, int cap) {
  lst->arr = realloc(lst->arr, cap * sizeof(void*));
  lst->cap = cap;
}

List* newList() {
  List* res = MALLOC(List);
  res->size = 0;
  res->cap = 1;
  res->arr = MALLOC(void*);
  return res;
}

void listPush(List* lst, void* e) {
  if (lst->cap == lst->size) {
    resize(lst, lst->cap * 2);
  }
  lst->arr[lst->size++] = e;
}

void* listGet(List* lst, int idx) {
  return lst->arr[idx];
}

int listSize(List* lst) {
  return lst->size;
}

void* listPop(List* lst){
  return lst->arr[--lst->size];
}

void* listLast(List* lst){
  assert(lst->size>0);
  return lst->arr[lst->size - 1];
}

void listSet(List* lst, int idx, void* v) {
  assert(idx < lst->size && idx >= 0);
  lst->arr[idx] = v;
}

List* listCopy(List* lst){
  List* res = newList();
  int i, len = listSize(lst);
  for(i=0;i<len;i++)
    listPush(res, listGet(lst, i));
  return res;
}

void listClear(List* lst){
  lst->size=0;
}
