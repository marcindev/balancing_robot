#include "testFixture.h"


void* pvPortMalloc(size_t size)
{
    return TestFixture::_rtosMock->pvPortMalloc(size);
}


void vPortFree(void* ptr)
{
    return TestFixture::_rtosMock->vPortFree(ptr);
}
