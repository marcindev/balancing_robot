#ifndef GRAPH_MOCK_H
#define GRAPH_MOCK_H

#include "gtest/gtest.h"
//#include "cmock/cmock.h"
#include "graph.h"
#include "baseMock.h"


class GraphMock : public BaseMock
{
public:

    GraphMock() { }
    virtual ~GraphMock() { }

    MOCK_METHOD1(GraphCreateNode, GraphNode(void*));
    MOCK_METHOD3(GraphCreateEdge, GraphEdge(GraphNode, GraphNode, void*));
    MOCK_METHOD2(GraphSetEdgeData, void(GraphEdge, void*));
    MOCK_METHOD1(GraphGetEdgeData, void*(GraphEdge));
    MOCK_METHOD2(GraphSetNodeData, void(GraphNode, void*));
    MOCK_METHOD1(GraphGetNodeData, void*(GraphNode));
    MOCK_METHOD2(GraphCreate, Graph(GraphEdge, GraphCompareFunc));
    MOCK_METHOD2(GraphInsertEdge, bool(Graph, GraphEdge));
    MOCK_METHOD2(GraphSetCurrentNode, bool(Graph, GraphNode));
    MOCK_METHOD1(GraphGetCurrentNode, GraphNode(Graph));
    MOCK_METHOD2(GraphSetCompareFunc, void(Graph, GraphCompareFunc));
    MOCK_METHOD2(GraphGoToNextNode, bool(Graph, void*));
    MOCK_METHOD3(GraphFindNode, bool(Graph, void*, GraphNode*));
    MOCK_METHOD1(GraphDestroy, void(Graph));

};

#endif // GRAPH_MOCK_H
