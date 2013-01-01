#ifndef _LIST_H_
#define _LIST_H_

#include "tl.h"
// list of node
typedef struct List {
  Node** arr;
  int size, cap;
} List;

List* newList();
void listAdd(List* lst, Node* e);
Node* listGet(List* lst, int idx);
int listSize(List* lst);

#endif
