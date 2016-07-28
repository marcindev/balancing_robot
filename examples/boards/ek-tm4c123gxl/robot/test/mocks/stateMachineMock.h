#ifndef STATE_MACHINE_MOCK_H
#define STATE_MACHINE_MOCK_H

#include "gtest/gtest.h"
//#include "cmock/cmock.h"
#include "stateMachine.h"
#include "baseMock.h"


class StateMachineMock : public BaseMock
{
public:
    StateMachineMock()
    {
    }

    virtual ~StateMachineMock()
    {
    }

    MOCK_METHOD0(fsmCreateSm, StateMachine());
    MOCK_METHOD1(fsmCreateState, SmState(FsmAction));
    MOCK_METHOD4(fsmAddEdge, bool(StateMachine, SmState, SmState, SmValue*));
    MOCK_METHOD2(fsmUpdate, void(StateMachine, SmValue));
    MOCK_METHOD2(fsmSetCurrentState, bool(StateMachine, SmState));
    MOCK_METHOD1(fsmDestroy, void(StateMachine));
    
};



#endif // STATE_MACHINE_MOCK_H
