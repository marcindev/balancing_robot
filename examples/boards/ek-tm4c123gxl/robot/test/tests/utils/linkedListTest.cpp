#include "gtest/gtest.h"
#include <stdlib.h>
#include "gmock/gmock.h"
extern "C" {
#include "linkedList.c"
}
#include "testFixture.h"
#include "helper.h"
#include <vector>

using ::testing::_;
using ::testing::Return;
using ::testing::InSequence;
using ::testing::Invoke;

namespace RobotTest {

class LinkedListTest : public TestFixture, public Helper
{
public:
    LinkedListTest()
    {
	ON_CALL(*_rtosMock, pvPortMalloc(_))
	    .WillByDefault(Invoke(this, &LinkedListTest::mallocWrapper));
	ON_CALL(*_rtosMock, vPortFree(_))
	    .WillByDefault(Invoke(this, &LinkedListTest::freeWrapper));

   
    }

    ~LinkedListTest()
    {
    
    }


};

TEST_F(LinkedListTest, ListCreateEmpty_createNewList_allocatesNewList)
{
    EXPECT_CALL(*_rtosMock, pvPortMalloc(_)).WillOnce(Invoke(this, &LinkedListTest::mallocWrapper));

    LinkedList list = ListCreateEmpty();
    EXPECT_NE(0, reinterpret_cast<int>(list));
}

TEST_F(LinkedListTest, ListInsert_elementInserted_sizeIncreasesByOne)
{
    EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	.Times(3)
	.WillRepeatedly(Invoke(this, &LinkedListTest::mallocWrapper));

    int data = 123;

    LinkedList list = ListCreateEmpty();
    EXPECT_EQ(0, ListSize(list));
    ListInsert(list, &data);
    EXPECT_EQ(1, ListSize(list));
    ListInsert(list, &data);
    EXPECT_EQ(2, ListSize(list));
}

TEST_F(LinkedListTest, ListRemove_listEmpty_fails)
{
    int data = 123;

    LinkedList list = ListCreateEmpty();
    
    EXPECT_FALSE(ListRemove(list, &data));
}

TEST_F(LinkedListTest, ListRemove_listNotEmptyGIvenElemNotInList_fails)
{
    int data1 = 1,
        data2 = 2,
        data3 = 3;

    LinkedList list = ListCreateEmpty();
    EXPECT_EQ(0, ListSize(list));
    ListInsert(list, &data1);
    EXPECT_EQ(1, ListSize(list));
    ListInsert(list, &data2);
    EXPECT_EQ(2, ListSize(list));
    ListInsert(list, &data3);
    EXPECT_EQ(3, ListSize(list));
  
    int data = 5;

    EXPECT_FALSE(ListRemove(list, &data));

    EXPECT_EQ(3, ListSize(list));
}

TEST_F(LinkedListTest, ListRemove_elemIsInList_succeeds)
{
    EXPECT_CALL(*_rtosMock, vPortFree(_))
	.Times(4)
	.WillRepeatedly(Invoke(this, &LinkedListTest::freeWrapper));

    int data1 = 1,
        data2 = 2,
        data3 = 3,
        data4 = 4;

    LinkedList list = ListCreateEmpty();
    EXPECT_EQ(0, ListSize(list));
    ListInsert(list, &data1);
    EXPECT_EQ(1, ListSize(list));
    ListInsert(list, &data2);
    EXPECT_EQ(2, ListSize(list));
    ListInsert(list, &data3);
    EXPECT_EQ(3, ListSize(list));
    ListInsert(list, &data4);
    EXPECT_EQ(4, ListSize(list));
  
    EXPECT_TRUE(ListRemove(list, &data2));
    EXPECT_EQ(3, ListSize(list));
    EXPECT_TRUE(ListRemove(list, &data4));
    EXPECT_EQ(2, ListSize(list));
    EXPECT_TRUE(ListRemove(list, &data1));
    EXPECT_EQ(1, ListSize(list));
    EXPECT_TRUE(ListRemove(list, &data3));
    EXPECT_EQ(0, ListSize(list));
    EXPECT_TRUE(ListIsEmpty(list));

}

TEST_F(LinkedListTest, ListGetNext_listIsEMpty_returnsNullPtr)
{

    LinkedList list = ListCreateEmpty();
 
    EXPECT_EQ(0, ListGetNext(list));
}

TEST_F(LinkedListTest, ListGetNext_listIsEMpty_fails)
{

    int data1 = 1,
        data2 = 2,
        data3 = 3,
        data4 = 4;

    LinkedList list = ListCreateEmpty();
    ListInsert(list, &data1);
    ListInsert(list, &data2);
    ListInsert(list, &data3);
    ListInsert(list, &data4);
 
    EXPECT_EQ(1, *(static_cast<int*>(ListGetNext(list))));
    EXPECT_EQ(2, *(static_cast<int*>(ListGetNext(list))));
    EXPECT_EQ(3, *(static_cast<int*>(ListGetNext(list))));
    EXPECT_EQ(4, *(static_cast<int*>(ListGetNext(list))));
    EXPECT_EQ(0, ListGetNext(list));

}

TEST_F(LinkedListTest, ListClear_listNotEmpty_makesListEmptyAndFreesMemory)
{

    int data1 = 1,
        data2 = 2,
        data3 = 3,
        data4 = 4;

    LinkedList list = ListCreateEmpty();
    ListInsert(list, &data1);
    ListInsert(list, &data2);
    ListInsert(list, &data3);
    ListInsert(list, &data4);

    EXPECT_FALSE(ListIsEmpty(list));
    ListClear(list);
    EXPECT_TRUE(ListIsEmpty(list));
    ListDestroy(list);
    EXPECT_EQ(0, getAllocCnt());

}

TEST_F(LinkedListTest, ListFind_listEmpty_returnsFalse)
{
    LinkedList list = ListCreateEmpty();
    
    int data = 123;

    EXPECT_FALSE(ListFind(list, &data));
    
}

TEST_F(LinkedListTest, ListFind_givenDataNotInList_returnsFalse)
{
    int data1 = 1,
        data2 = 2,
        data3 = 3,
        data4 = 4;

    LinkedList list = ListCreateEmpty();
    ListInsert(list, &data1);
    ListInsert(list, &data2);
    ListInsert(list, &data3);
    ListInsert(list, &data4);
  
    int dataToFind = 123;

    EXPECT_FALSE(ListFind(list, &dataToFind));

}

TEST_F(LinkedListTest, ListFind_givenDataIsInList_returnsTrue)
{
    int data1 = 1,
        data2 = 2,
        data3 = 3,
        data4 = 4;

    LinkedList list = ListCreateEmpty();
    ListInsert(list, &data1);
    ListInsert(list, &data2);
    ListInsert(list, &data3);
    ListInsert(list, &data4);
  

    EXPECT_TRUE(ListFind(list, &data2));
    EXPECT_TRUE(ListFind(list, &data4));
    EXPECT_TRUE(ListFind(list, &data1));
    EXPECT_TRUE(ListFind(list, &data3));

}

TEST_F(LinkedListTest, ListDestroy_clearsList_allAllocatedMemoryIsFreed)
{
    int data1 = 1,
        data2 = 2,
        data3 = 3,
        data4 = 4;

    LinkedList list = ListCreateEmpty();
    ListInsert(list, &data1);
    ListInsert(list, &data2);
    ListInsert(list, &data3);
    ListInsert(list, &data4);

    ListDestroy(list);

    EXPECT_EQ(0, getAllocCnt());
}



}
