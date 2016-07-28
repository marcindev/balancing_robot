#include "linkedList.h"
#include "FreeRTOS.h"
#include "portable.h"

typedef struct _Node
{
    void* data;
    struct _Node* next;
} Node;

struct _LinkedList
{
    Node* beg;
    Node* end;
    Node* iter;
    uint32_t size;
};

LinkedList ListCreateEmpty()
{
    LinkedList list = (LinkedList) pvPortMalloc(sizeof(struct _LinkedList));
    list->beg = NULL;
    list->end = NULL;
    list->iter = list->beg;
    list->size = 0;

    return list;
}

void ListInsert(LinkedList list, void* data)
{
   Node* newNode = (Node*) pvPortMalloc(sizeof(Node));
   newNode->data = data;
   newNode->next = NULL;

   if(!list->beg)
   {
       list->beg = newNode;
       list->end = list->beg;
       list->iter = list->beg;
   }
   else
   {
       list->end->next = newNode;
       list->end = newNode;
   }

   list->size++;
}

static Node* getNextNode(LinkedList list)
{
    Node* nextNode = list->iter;

    if(nextNode)
	list->iter = list->iter->next;

    return nextNode;
}

uint32_t ListSize(LinkedList list)
{
    return list->size;
}

bool ListIsEmpty(LinkedList list)
{
    return !list->size;
}
 

bool ListRemove(LinkedList list, void* data)
{
    ListIterReset(list);
    Node* prevNode = NULL;

    while(true)
    {
	Node* nextNode = getNextNode(list);

	if(!nextNode) return false;

	if(nextNode->data == data)
	{
	    if(prevNode)
	    {
		prevNode->next = nextNode->next;
	    }
	    else
	    {
		list->beg = nextNode->next;	
	    }

	    vPortFree(nextNode);
	    break;
	}

	prevNode = nextNode;
    }

    list->size--;
    return true;
}

void ListIterReset(LinkedList list)
{
    list->iter = list->beg;   
}

void* ListGetNext(LinkedList list)
{
    Node* node = getNextNode(list);

    if(!node) return NULL;

    return node->data;
}
 
void ListClear(LinkedList list)
{
    ListIterReset(list);
    
    while(true)
    {
	void* data = ListGetNext(list);
	if(!ListRemove(list, data)) return;
	
	ListIterReset(list);
    }
}

bool ListFind(LinkedList list, void* data)
{
    ListIterReset(list);
    
    while(true)
    {
	void* next = ListGetNext(list);

	if(next == data)
	    return true;
	else if(next == NULL)
	    break;
	
    }

    return false;
}   

void ListDestroy(LinkedList list)
{
    ListClear(list);
    
    vPortFree(list);
}
