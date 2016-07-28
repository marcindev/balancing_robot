#include "testFixture.h"

extern "C" {


LinkedList __real_ListCreateEmpty();
void __real_ListInsert(LinkedList list, void* data);
bool __real_ListRemove(LinkedList list, void* data);
void __real_ListIterReset(LinkedList list);
void* __real_ListGetNext(LinkedList list);
uint32_t __real_ListSize(LinkedList list);
bool __real_ListIsEmpty(LinkedList list);
void __real_ListClear(LinkedList list);
bool __real_ListFind(LinkedList list, void* data);
void __real_ListDestroy(LinkedList list);



LinkedList __wrap_ListCreateEmpty()
{
    if(TestFixture::_linkedListMock->isEnabled())
        return TestFixture::_linkedListMock->ListCreateEmpty();

    return __real_ListCreateEmpty();
}

void __wrap_ListInsert(LinkedList list, void* data)
{
    if(TestFixture::_linkedListMock->isEnabled())
        TestFixture::_linkedListMock->ListInsert(list, data);
    else
    __real_ListInsert(list, data);
}

bool __wrap_ListRemove(LinkedList list, void* data)
{
    if(TestFixture::_linkedListMock->isEnabled())
        return TestFixture::_linkedListMock->ListRemove(list, data);

    return __real_ListRemove(list, data);
}

void __wrap_ListIterReset(LinkedList list)
{
    if(TestFixture::_linkedListMock->isEnabled())
        TestFixture::_linkedListMock->ListIterReset(list);
    else
    __real_ListIterReset(list);
}

void* __wrap_ListGetNext(LinkedList list)
{
    if(TestFixture::_linkedListMock->isEnabled())
        return TestFixture::_linkedListMock->ListGetNext(list);

    return __real_ListGetNext(list);
}

uint32_t __wrap_ListSize(LinkedList list)
{
    if(TestFixture::_linkedListMock->isEnabled())
        return TestFixture::_linkedListMock->ListSize(list);

    return __real_ListSize(list);
}

bool __wrap_ListIsEmpty(LinkedList list)
{
    if(TestFixture::_linkedListMock->isEnabled())
        return TestFixture::_linkedListMock->ListIsEmpty(list);

    return __real_ListIsEmpty(list);
}

void __wrap_ListClear(LinkedList list)
{
    if(TestFixture::_linkedListMock->isEnabled())
        TestFixture::_linkedListMock->ListClear(list);
    else
     __real_ListClear(list);
}

bool __wrap_ListFind(LinkedList list, void* data)
{
    if(TestFixture::_linkedListMock->isEnabled())
        return TestFixture::_linkedListMock->ListFind(list, data);

    return __real_ListFind(list, data);
}

void __wrap_ListDestroy(LinkedList list)
{
    if(TestFixture::_linkedListMock->isEnabled())
	 TestFixture::_linkedListMock->ListDestroy(list);
    else
     __real_ListDestroy(list);
}


}

