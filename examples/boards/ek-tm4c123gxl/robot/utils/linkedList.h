#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdint.h>
#include <stdbool.h>

typedef struct _LinkedList* LinkedList;

LinkedList ListCreateEmpty();
void ListInsert(LinkedList list, void* data);
bool ListRemove(LinkedList list, void* data);
void ListIterReset(LinkedList list);
void* ListGetNext(LinkedList list);
uint32_t ListSize(LinkedList list);
bool ListIsEmpty(LinkedList list);
void ListClear(LinkedList list);
bool ListFind(LinkedList list, void* data);
void ListDestroy(LinkedList list);    

#endif // LINKED_LIST_H
