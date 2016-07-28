#include "gtest/gtest.h"
#include <stdlib.h>
#include "gmock/gmock.h"
//#ifndef _UT
//#define _UT
//#endif
extern "C" {
#include "stateMachine.c"
}
#include "graphMock.h"
#include "testFixture.h"
#include <vector>

using ::testing::_;
using ::testing::Return;
using ::testing::InSequence;
using ::testing::Invoke;

namespace RobotTest {

class StateMachineTest : public TestFixture
{
public:
    StateMachineTest()
    {
	// Here can be placed initialization of globals etc.
	sm = &_sm;
	state = &_state;
	actionCallCnt = 0;
	ON_CALL(*_graphMock, GraphCreateNode(_))
	    .WillByDefault(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<GraphNode>));

	ON_CALL(*_graphMock, GraphCreateEdge(_,_,_))
	    .WillByDefault(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<GraphEdge>));

	ON_CALL(*_graphMock, GraphCreate(_,_))
	    .WillByDefault(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<Graph>));

	ON_CALL(*_graphMock, GraphFindNode(_,_,_)).WillByDefault(Return(true));

    }

    ~StateMachineTest()
    {
	cleanObjects();
    }


    static void smAction() { ++actionCallCnt; }
    
    template <typename T>
    T allocNextObject();

    static int actionCallCnt;    
    void cleanObjects();
    std::vector<void*> objectPtrs;

    _StateMachine _sm;
    StateMachine sm;
    _SmState _state;
    SmState state;
};


int StateMachineTest::actionCallCnt = 0;


void StateMachineTest::cleanObjects()
{
    for(auto& ptr : objectPtrs)
	free(ptr);

    objectPtrs.clear();
}

template <typename T>
T StateMachineTest::allocNextObject()
{
    static T obj = reinterpret_cast<T>(12345);
    obj = reinterpret_cast<T>(reinterpret_cast<int>(obj) + 1);
    return obj;
}

template <>
SmState StateMachineTest::allocNextObject<SmState>()
{
    SmState st = static_cast<SmState>(malloc(sizeof(struct _SmState)));
    objectPtrs.push_back(st); 

    return st;
}

template <>
StateMachine StateMachineTest::allocNextObject<StateMachine>()
{
    StateMachine sm = static_cast<StateMachine>(malloc(sizeof(struct _StateMachine)));
    objectPtrs.push_back(sm); 

    return sm;
}



TEST_F(StateMachineTest, fsmCreateSm_create_success)
{
    EXPECT_CALL(*_rtosMock, pvPortMalloc(_)).WillOnce(Return(sm));

    EXPECT_EQ(sm, fsmCreateSm());
}

TEST_F(StateMachineTest, fsmCreateState_create_success)
{
    EXPECT_CALL(*_rtosMock, pvPortMalloc(_)).WillOnce(Return(state));

    ASSERT_EQ(state, fsmCreateState(smAction));
    ASSERT_EQ(smAction, state->doAction);
    
    ASSERT_FALSE(actionCallCnt);
    state->doAction();
    EXPECT_TRUE(actionCallCnt);
}

TEST_F(StateMachineTest, fsmAddEdge_firstEdge_success)
{
    SmValue transVal = 1;

    {
	InSequence seq;

	EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	    .Times(2)
	    .WillRepeatedly(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<SmState>));

	EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	    .WillOnce(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<StateMachine>));
	
    }

    EXPECT_CALL(*_graphMock, GraphCreateNode(_))
	.Times(2)
	.WillRepeatedly(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<GraphNode>));

    EXPECT_CALL(*_graphMock, GraphCreateEdge(_,_,&transVal))
	.WillOnce(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<GraphEdge>));
    EXPECT_CALL(*_graphMock, GraphCreate(_,_))
	.WillOnce(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<Graph>));

    SmState firstState = fsmCreateState(smAction),
	    secondState = fsmCreateState(smAction);
  
    StateMachine sm = fsmCreateSm(); 

    EXPECT_TRUE(fsmAddEdge(sm, firstState, secondState, &transVal));
   
}


TEST_F(StateMachineTest, fsmAddEdge_notConnectedEdges_fails)
{
    SmValue transVal = 1;

    {
	InSequence seq;

	EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	    .Times(4)
	    .WillRepeatedly(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<SmState>));

	EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	    .WillOnce(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<StateMachine>));
	
    }

    EXPECT_CALL(*_graphMock, GraphCreateNode(_))
	.Times(2)
	.WillRepeatedly(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<GraphNode>));

    EXPECT_CALL(*_graphMock, GraphCreateEdge(_,_,&transVal))
	.Times(1)
	.WillRepeatedly(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<GraphEdge>));
    EXPECT_CALL(*_graphMock, GraphCreate(_,_))
	.WillOnce(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<Graph>));

    EXPECT_CALL(*_graphMock, GraphFindNode(_,_,_)).WillOnce(Return(false));

    SmState firstState = fsmCreateState(smAction),
	    secondState = fsmCreateState(smAction),
	    thirdState = fsmCreateState(smAction),
	    fourthState = fsmCreateState(smAction);
  
    StateMachine sm = fsmCreateSm(); 

    EXPECT_TRUE(fsmAddEdge(sm, firstState, secondState, &transVal));
    EXPECT_FALSE(fsmAddEdge(sm, thirdState, fourthState, &transVal));
   
}

TEST_F(StateMachineTest, fsmUpdate_failsOnGoToNextNode_actionIsNotCalled)
{
    SmValue transVal = 1,
	    input = 1;

    {
	InSequence seq;

	EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	    .Times(4)
	    .WillRepeatedly(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<SmState>));

	EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	    .WillOnce(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<StateMachine>));
	
    }

    EXPECT_CALL(*_graphMock, GraphGoToNextNode(_,_)).WillOnce(Return(false));

    SmState firstState = fsmCreateState(smAction),
	    secondState = fsmCreateState(smAction),
	    thirdState = fsmCreateState(smAction),
	    fourthState = fsmCreateState(smAction);
  
    StateMachine sm = fsmCreateSm(); 

    fsmAddEdge(sm, firstState, secondState, &transVal);
    fsmAddEdge(sm, secondState, thirdState, &transVal);
    fsmAddEdge(sm, thirdState, fourthState, &transVal);

    ASSERT_EQ(0, actionCallCnt); 

    fsmUpdate(sm, input);

    EXPECT_EQ(0, actionCallCnt); 
}

TEST_F(StateMachineTest, fsmUpdate_succeedsOnGoToNextNode_actionIsCalled)
{
    SmValue transVal = 1,
	    input = 1;

    {
	InSequence seq;

	EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	    .Times(4)
	    .WillRepeatedly(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<SmState>));

	EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	    .WillOnce(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<StateMachine>));
	
    }

    EXPECT_CALL(*_graphMock, GraphGoToNextNode(_,_))
	.Times(2)
	.WillRepeatedly(Return(true));

    EXPECT_CALL(*_graphMock, GraphGetCurrentNode(_))
	.Times(2)
	.WillRepeatedly(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<GraphNode>));

    SmState firstState = fsmCreateState(smAction),
	    secondState = fsmCreateState(smAction),
	    thirdState = fsmCreateState(smAction),
	    fourthState = fsmCreateState(smAction);
  
    EXPECT_CALL(*_graphMock, GraphGetNodeData(_))
	.WillOnce(Return(secondState))
	.WillOnce(Return(thirdState));



    StateMachine sm = fsmCreateSm(); 

    fsmAddEdge(sm, firstState, secondState, &transVal);
    fsmAddEdge(sm, secondState, thirdState, &transVal);
    fsmAddEdge(sm, thirdState, fourthState, &transVal);

    ASSERT_EQ(0, actionCallCnt); 

    fsmUpdate(sm, input);

    EXPECT_EQ(1, actionCallCnt); 
    
    fsmUpdate(sm, input);

    EXPECT_EQ(2, actionCallCnt); 
}

TEST_F(StateMachineTest, fsmSetCurrentState_nodeNotFound_fails)
{
    {
	InSequence seq;

	EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	    .WillOnce(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<SmState>));

	EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	    .WillOnce(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<StateMachine>));
	
    }

    EXPECT_CALL(*_graphMock, GraphFindNode(_,_,_)).WillOnce(Return(false));

    SmState firstState = fsmCreateState(smAction);
    StateMachine sm = fsmCreateSm(); 
    
    EXPECT_FALSE(fsmSetCurrentState(sm, firstState));
}

TEST_F(StateMachineTest, fsmSetCurrentState_nodeFound_succeeds)
{
    {
	InSequence seq;

	EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	    .WillOnce(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<SmState>));

	EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	    .WillOnce(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<StateMachine>));
	
    }

    EXPECT_CALL(*_graphMock, GraphFindNode(_,_,_)).WillOnce(Return(true));
    EXPECT_CALL(*_graphMock, GraphSetCurrentNode(_,_)).WillOnce(Return(true));

    SmState firstState = fsmCreateState(smAction);
    StateMachine sm = fsmCreateSm(); 
    
    EXPECT_TRUE(fsmSetCurrentState(sm, firstState));
}

TEST_F(StateMachineTest, fsmDestroy_destroy_graphIsDestroyedAndMemmoryDeallocated)
{
    EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	    .WillOnce(InvokeWithoutArgs(this, &StateMachineTest::allocNextObject<StateMachine>));

    EXPECT_CALL(*_rtosMock, vPortFree(_)).Times(1);

    EXPECT_CALL(*_graphMock, GraphDestroy(_)).Times(1);

    StateMachine sm = fsmCreateSm(); 
   
    fsmDestroy(sm);
}



}
