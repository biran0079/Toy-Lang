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
