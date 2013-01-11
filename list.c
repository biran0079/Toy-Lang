#include "list.h"
#include "util.h"

static void resize(List* lst, int cap) {
  lst->arr = realloc(lst->arr, cap * sizeof(void*));
  lst->cap = cap;
}

extern int newListC, freeListC;

List* newList() {
  newListC++;
  List* res = MALLOC(List);
  res->size = 0;
  res->cap = 1;
  res->arr = MALLOC(void*);
  return res;
}

void freeList(List* lst) {
  freeListC++;
  int i;
  free(lst->arr);
  lst->arr = 0;
  lst-> cap = 0;
  lst-> size = 0;
  free(lst);
}

void listPush(List* lst, void* e) {
  if (lst->cap == lst->size) {
    resize(lst, lst->cap * 2);
  }
  lst->arr[lst->size++] = e;
}

void* listGet(List* lst, int idx) {
  if(idx >= lst->size || idx < 0)
    error("list index out of boundary");
  return lst->arr[idx];
}

int listSize(List* lst) {
  return lst->size;
}

void* listPop(List* lst){
  if(lst->size == 0) error("cannot pop empty list\n");
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
  
static void swap(void* a[], int i, int j) {
  if(i==j) return;
  void* t = a[i];
  a[i] = a[j];
  a[j] = t;
}

static void qsortInternal(void* a[], int l, int r, Comparator cmp) {
  if (l>=r) return;
  int i = l+1, j = l+1;
  while(i <= r) {
    if(cmp(a[i], a[l]) < 0)
      swap(a, i, j++);
    i++;
  }
  j--;
  swap(a, l, j);
  qsortInternal(a, l, j-1, cmp);
  qsortInternal(a, j+1, r, cmp);
}

void listSort(List* lst, Comparator cmp) {
  qsortInternal(lst->arr, 0, lst->size - 1, cmp);
}
