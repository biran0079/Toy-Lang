#include "list.h"

static void resize(List* lst, int cap) {
  lst->arr = (Node**)realloc(lst->arr, cap * sizeof(Node**));
  lst->cap = cap;
}

List* newList() {
  List* res = MALLOC(List);
  res->size = 0;
  res->cap = 1;
  res->arr = MALLOC(Node*);
}

void listAdd(List* lst, Node* e) {
  if (lst->cap == lst->size) {
    resize(lst, lst->cap * 2);
  }
  lst->arr[lst->size++] = e;
}

Node* listGet(List* lst, int idx) {
  return lst->arr[idx];
}

int listSize(List* lst) {
  return lst->size;
}
