#ifndef LINKED_LIST_MOCK_H
#define LINKED_LIST_MOCK_H

#include "gtest/gtest.h"
#include "linkedList.h"
#include "baseMock.h"

class LinkedListMock : public BaseMock
{
public:
    LinkedListMock() { }
    virtual ~LinkedListMock() { }

    MOCK_METHOD0(ListCreateEmpty, LinkedList());
    MOCK_METHOD2(ListInsert, void(LinkedList, void*));
    MOCK_METHOD2(ListRemove, bool(LinkedList, void*));
    MOCK_METHOD1(ListIterReset, void(LinkedList));
    MOCK_METHOD1(ListGetNext, void*(LinkedList));
    MOCK_METHOD1(ListSize, uint32_t(LinkedList));
    MOCK_METHOD1(ListIsEmpty, bool(LinkedList));
    MOCK_METHOD1(ListClear, void(LinkedList));
    MOCK_METHOD2(ListFind, bool(LinkedList, void*));
    MOCK_METHOD1(ListDestroy, void(LinkedList));
};

#endif // LINKED_LIST_MOCK_H
