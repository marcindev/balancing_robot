#ifndef RTOS_MOCK_H
#define RTOS_MOCK_H

#include "gtest/gtest.h"
#include "FreeRTOS.h"

class RtosMock
{
public:
    RtosMock() {}
    virtual ~RtosMock() {}

    MOCK_METHOD1(pvPortMalloc, void*(size_t));
    MOCK_METHOD1(vPortFree, void(void*));

};

#endif // RTOS_MOCK_H
