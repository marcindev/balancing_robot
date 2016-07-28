#include "testFixture.h"


extern "C" {

GraphNode __real_GraphCreateNode(void* data);
GraphEdge __real_GraphCreateEdge(GraphNode firstNode, GraphNode secondNode, void* data);
void __real_GraphSetEdgeData(GraphEdge edge, void* data);
void* __real_GraphGetEdgeData(GraphEdge edge);
void* __real_GraphGetEdgeData(GraphEdge edge);
void __real_GraphSetNodeData(GraphNode, void* data);
void* __real_GraphGetNodeData(GraphNode node);
Graph __real_GraphCreate(GraphEdge edge, GraphCompareFunc func);
bool __real_GraphInsertEdge(Graph graph, GraphEdge edge);
bool __real_GraphSetCurrentNode(Graph graph, GraphNode node);
GraphNode __real_GraphGetCurrentNode(Graph graph);
void __real_GraphSetCompareFunc(Graph graph, GraphCompareFunc func);
bool __real_GraphGoToNextNode(Graph graph, void* input);
bool __real_GraphFindNode(Graph graph, void* data, GraphNode* node);
void __real_GraphDestroy(Graph graph);


GraphNode __wrap_GraphCreateNode(void* data)
{
    if(TestFixture::_graphMock->isEnabled())
        return TestFixture::_graphMock->GraphCreateNode(data);

    return __real_GraphCreateNode(data);
}

GraphEdge __wrap_GraphCreateEdge(GraphNode firstNode, GraphNode secondNode, void* data)
{
    if(TestFixture::_graphMock->isEnabled())
        return TestFixture::_graphMock->GraphCreateEdge(firstNode, secondNode, data);

    return __real_GraphCreateEdge(firstNode, secondNode, data);
}

void __wrap_GraphSetEdgeData(GraphEdge edge, void* data)
{
    if(TestFixture::_graphMock->isEnabled())
        TestFixture::_graphMock->GraphSetEdgeData(edge, data);
    else
	 __real_GraphSetEdgeData(edge, data);
}

void* __wrap_GraphGetEdgeData(GraphEdge edge)
{
    if(TestFixture::_graphMock->isEnabled())
        return TestFixture::_graphMock->GraphGetEdgeData(edge);

    return __real_GraphGetEdgeData(edge);
}

void __wrap_GraphSetNodeData(GraphNode node, void* data)
{
    if(TestFixture::_graphMock->isEnabled())
        TestFixture::_graphMock->GraphSetNodeData(node, data);
    else
    __real_GraphSetNodeData(node, data);
}

void* __wrap_GraphGetNodeData(GraphNode node)
{
    if(TestFixture::_graphMock->isEnabled())
        return TestFixture::_graphMock->GraphGetNodeData(node);

    return __real_GraphGetNodeData(node);
}

Graph __wrap_GraphCreate(GraphEdge edge, GraphCompareFunc func)
{
    if(TestFixture::_graphMock->isEnabled())
        return TestFixture::_graphMock->GraphCreate(edge, func);

    return __real_GraphCreate(edge, func);
}

bool __wrap_GraphInsertEdge(Graph graph, GraphEdge edge)
{
    if(TestFixture::_graphMock->isEnabled())
        return TestFixture::_graphMock->GraphInsertEdge(graph, edge);

    return __real_GraphInsertEdge(graph, edge);
}

bool __wrap_GraphSetCurrentNode(Graph graph, GraphNode node)
{
    if(TestFixture::_graphMock->isEnabled())
        return TestFixture::_graphMock->GraphSetCurrentNode(graph, node);

    return __real_GraphSetCurrentNode(graph, node);
}

GraphNode __wrap_GraphGetCurrentNode(Graph graph)
{
    if(TestFixture::_graphMock->isEnabled())
        return TestFixture::_graphMock->GraphGetCurrentNode(graph);

    return __real_GraphGetCurrentNode(graph);
}

void __wrap_GraphSetCompareFunc(Graph graph, GraphCompareFunc func)
{
    if(TestFixture::_graphMock->isEnabled())
        TestFixture::_graphMock->GraphSetCompareFunc(graph, func);
    else 
        __real_GraphSetCompareFunc(graph, func);
}

bool __wrap_GraphGoToNextNode(Graph graph, void* input)
{
    if(TestFixture::_graphMock->isEnabled())
        return TestFixture::_graphMock->GraphGoToNextNode(graph, input);

    return __real_GraphGoToNextNode(graph, input);
}

bool __wrap_GraphFindNode(Graph graph, void* data, GraphNode* node)
{
    if(TestFixture::_graphMock->isEnabled())
        return TestFixture::_graphMock->GraphFindNode(graph, data, node);

    return __real_GraphFindNode(graph, data, node);
}

void __wrap_GraphDestroy(Graph graph)
{
    if(TestFixture::_graphMock->isEnabled())
        TestFixture::_graphMock->GraphDestroy(graph);
    else
        __real_GraphDestroy(graph);
}


}
