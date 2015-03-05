#include "list.h"
#include "util.h"

static void resize(List *lst, int cap) {
  lst->arr = tlRealloc(lst->arr, cap * sizeof(void *));
  lst->cap = cap;
}

int newListC = 0, freeListC = 0;

#define INIT_LIST_CAP 1

List *newList() {
  newListC++;
  List *res = MALLOC(List);
  res->size = 0;
  res->cap = INIT_LIST_CAP;
  res->arr = tlMalloc(res->cap * sizeof(void *));
  return res;
}

void freeList(List *lst) {
  freeListC++;
  tlFree(lst->arr);
  lst->arr = 0;
  lst->cap = 0;
  lst->size = 0;
  tlFree(lst);
}

void listPush(List *lst, void *e) {
  if (lst->cap == lst->size) {
    resize(lst, lst->cap * 2);
  }
  lst->arr[lst->size++] = e;
}

void listPushFront(List *lst, void *e) {
  if (lst->cap == lst->size) {
    resize(lst, lst->cap * 2);
  }
  int i;
  for (i = lst->size; i > 0; i--) {
    lst->arr[i] = lst->arr[i - 1];
  }
  lst->arr[0] = e;
  lst->size++;
}

void *listPop(List *lst) {
  if (lst->size == 0) {
    error("cannot pop empty list\n");
  }
  return lst->arr[--lst->size];
}

void *listLast(List *lst) {
  assert(lst->size > 0);
  return lst->arr[lst->size - 1];
}

void *listSet(List *lst, int idx, void *v) {
  assert(idx < lst->size && idx >= 0);
  void *oldValue = lst->arr[idx];
  lst->arr[idx] = v;
  return oldValue;
}

List *listCopy(List *lst) {
  List *res = newList();
  int i, len = lst->size;
  for (i = 0; i < len; i++) listPush(res, listGet(lst, i));
  return res;
}

void listClear(List *lst) { lst->size = 0; }

static void msortInternal(void *a[], int l, int r, Comparator cmp, void *t[]) {
  if (r - l <= 1) return;
  int mid = (l + r) / 2;
  msortInternal(a, l, mid, cmp, t);
  msortInternal(a, mid, r, cmp, t);
  int k = l, i = l, j = mid;
  while (i < mid && j < r) {
    if (cmp(a[i], a[j]) <= 0)
      t[k++] = a[i++];
    else
      t[k++] = a[j++];
  }
  while (i < mid) t[k++] = a[i++];
  while (j < r) t[k++] = a[j++];
  for (i = l; i < r; i++) a[i] = t[i];
}

void listSort(List *lst, Comparator cmp) {
  void **t = tlMalloc(listSize(lst) * sizeof(void *));
  msortInternal(lst->arr, 0, lst->size, cmp, t);
  tlFree(t);
}

void listPopTo(List *lst, int size) {
  if (lst->size < size)
    error("size passed to listPopTo is less than list size\n");
  lst->size = size;
}

void increaseSizeTo(List *lst, int size) {
  if (size <= lst->size) return;
  int cap = lst->cap;
  if (cap < size) {
    while (cap < size) cap *= 2;
    resize(lst, cap);
  }
  memset(lst->arr + lst->size, 0, sizeof(void *) * (size - lst->size));
  lst->size = size;
}

void listSwap(List *lst, int i, int j) {
  if (i == j) return;
  void *t = listGet(lst, i);
  listSet(lst, i, listGet(lst, j));
  listSet(lst, j, t);
}
