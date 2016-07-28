#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct _StateMachine* StateMachine;
typedef struct _SmState* SmState;
typedef void(*FsmAction)();
typedef uint32_t SmValue;

StateMachine fsmCreateSm();
SmState fsmCreateState(FsmAction doAction);
bool fsmAddEdge(StateMachine sm,
		SmState firstState, 
		SmState secondState, 
		SmValue* transitionValue);
void fsmUpdate(StateMachine sm, SmValue input);
bool fsmSetCurrentState(StateMachine sm, SmState state);
void fsmDestroy(StateMachine sm);

#endif //STATE_MACHINE_H


