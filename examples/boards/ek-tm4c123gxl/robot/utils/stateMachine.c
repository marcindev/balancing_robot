#include "stateMachine.h"
#include "graph.h"
#include "FreeRTOS.h"
#include "portable.h"


struct _SmState
{
   FsmAction doAction; 
};

struct _StateMachine 
{
    Graph graph;
};

static bool compare(void* data1, void* data2)
{
    return *((SmValue*)data1) == *((SmValue*)data2);
}


StateMachine fsmCreateSm()
{
    StateMachine sm = (StateMachine) pvPortMalloc(sizeof(struct _StateMachine));
    sm->graph = NULL;
    
    return sm;
}

SmState fsmCreateState(FsmAction doAction)
{
    SmState state = (SmState) pvPortMalloc(sizeof(struct _SmState));
    state->doAction = doAction;

    return state;
}

bool fsmAddEdge(StateMachine sm,
		SmState firstState, 
		SmState secondState, 
		SmValue* transitionValue)
{
    GraphEdge edge;
    GraphNode fNode, sNode;

    if(!sm->graph)
    {
        fNode = GraphCreateNode(firstState);
	sNode = GraphCreateNode(secondState);
	edge = GraphCreateEdge(fNode, sNode, transitionValue);
	sm->graph = GraphCreate(edge, compare);
	return true;	
    }

    if(!GraphFindNode(sm->graph, &firstState, &fNode))
	return false;

    sNode = GraphCreateNode(secondState);
    edge = GraphCreateEdge(fNode, sNode, transitionValue);
    
    return GraphInsertEdge(sm->graph, edge);
}

void fsmUpdate(StateMachine sm, SmValue input)
{
    if(GraphGoToNextNode(sm->graph, &input))
    {
	GraphNode currNode = GraphGetCurrentNode(sm->graph);
	SmState currState = (SmState) GraphGetNodeData(currNode);
	currState->doAction();
    }
}

bool fsmSetCurrentState(StateMachine sm, SmState state)
{
    GraphNode node;

    if(!GraphFindNode(sm->graph, state, &node))
	return false;

    return GraphSetCurrentNode(sm->graph, node);
}

void fsmDestroy(StateMachine sm)
{
    GraphDestroy(sm->graph);
    vPortFree(sm);
}


