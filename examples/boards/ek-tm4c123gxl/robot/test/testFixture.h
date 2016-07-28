#ifndef TEST_FIXTURE_H
#define TEST_FIXTURE_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "rtosMock.h"
#include "stateMachineMock.h"
#include "graphMock.h"
#include "linkedListMock.h"
#include <memory>


class TestFixture : public ::testing::Test
{
public:
    TestFixture()
    {
	// alloc ptrs with nice mocks
	_stateMachineMock.reset(new ::testing::NiceMock<StateMachineMock>());
	_rtosMock.reset(new ::testing::NiceMock<RtosMock>());
	_graphMock.reset(new ::testing::NiceMock<GraphMock>());
	_linkedListMock.reset(new ::testing::NiceMock<LinkedListMock>());
    }

    ~TestFixture()
    {
	// reset ptrs
	_stateMachineMock.reset();
	_rtosMock.reset();
	_graphMock.reset();
    }

// TODO: place unique ptrs to mocks 
    static std::unique_ptr<StateMachineMock> _stateMachineMock;
    static std::unique_ptr<RtosMock> _rtosMock;
    static std::unique_ptr<GraphMock> _graphMock;
    static std::unique_ptr<LinkedListMock> _linkedListMock;
};

#endif // TEST_FIXTURE_H
