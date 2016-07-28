#include "testFixture.h"

extern "C" {

StateMachine __real_fsmCreateSm();
SmState __real_fsmCreateState(FsmAction action);
bool __real_fsmAddEdge(StateMachine sm, SmState st1, SmState st2, SmValue* val);
void __real_fsmUpdate(StateMachine sm, SmValue val);
bool __real_fsmSetCurrentState(StateMachine sm, SmState st);
void __real_fsmDestroy(StateMachine sm);


StateMachine __wrap_fsmCreateSm()
{
    if(TestFixture::_stateMachineMock->isEnabled())
	return TestFixture::_stateMachineMock->fsmCreateSm();

    return __real_fsmCreateSm();
}

SmState __wrap_fsmCreateState(FsmAction action)
{
    if(TestFixture::_stateMachineMock->isEnabled())
	return TestFixture::_stateMachineMock->fsmCreateState(action);

    return __real_fsmCreateState(action);
}

bool __wrap_fsmAddEdge(StateMachine sm, SmState st1, SmState st2, SmValue* val)
{
    if(TestFixture::_stateMachineMock->isEnabled())
	return TestFixture::_stateMachineMock->fsmAddEdge(sm, st1, st2, val);

    return __real_fsmAddEdge(sm, st1, st2, val);

}

void __wrap_fsmUpdate(StateMachine sm, SmValue val)
{
    if(TestFixture::_stateMachineMock->isEnabled())
	TestFixture::_stateMachineMock->fsmUpdate(sm, val);
    else
	 __real_fsmUpdate(sm, val);

}

bool __wrap_fsmSetCurrentState(StateMachine sm, SmState st)
{
    if(TestFixture::_stateMachineMock->isEnabled())
	return TestFixture::_stateMachineMock->fsmSetCurrentState(sm, st);

    return __real_fsmSetCurrentState(sm, st);

}

void __wrap_fsmDestroy(StateMachine sm)
{
    if(TestFixture::_stateMachineMock->isEnabled())
	TestFixture::_stateMachineMock->fsmDestroy(sm);
    else
	 __real_fsmDestroy(sm);

}


}

   


