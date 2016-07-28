#ifndef HELPER_H
#define HELPER_H
#include <stdlib.h>
#include <list>
#include "gtest/gtest.h"
#include <iostream>


namespace RobotTest {

class Helper
{
public:

    Helper() : allocCnt(0)
    {

    }

    ~Helper()
    {
	cleanObjects();
    }

    template <typename T>
    T allocNextObject();

    template <typename T>
    inline T returnNullPtr();

    inline void* mallocWrapper(size_t size);
    inline void freeWrapper(void* ptr);
    inline int getAllocCnt() { return allocCnt; }
    inline void printAllocs();
private:

    inline void cleanObjects();
    std::list<void*> objectPtrs;
    int allocCnt;

};


void Helper::cleanObjects()
{
    for(auto& ptr : objectPtrs)
	free(ptr);

    objectPtrs.clear();
}

template <typename T>
T Helper::allocNextObject()
{
    static T obj = reinterpret_cast<T>(12345);
    obj = reinterpret_cast<T>(reinterpret_cast<int>(obj) + 1);
    return obj;
}

template <typename T>
T Helper::returnNullPtr()
{
   return reinterpret_cast<T>(nullptr);
}

void* Helper::mallocWrapper(size_t size)
{
    ++allocCnt;
    void* ptr = malloc(size);
    objectPtrs.push_back(ptr);

    
    return ptr;
}

void Helper::freeWrapper(void* ptr)
{
    --allocCnt;
    objectPtrs.remove(ptr);
    free(ptr);
}

void Helper::printAllocs()
{
    std::cout << "Allocs: " << std::endl;
    for(auto& i : objectPtrs)
	std::cout << "0x" << std::hex << i << std::endl;
}


}

#endif // HELPER_H
