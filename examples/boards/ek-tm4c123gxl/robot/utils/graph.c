#include "graph.h"
#include "linkedList.h"
#include "FreeRTOS.h"
#include "portable.h"


struct _Node
{
    void* data;
    LinkedList edges;
    bool isInGraph;
}; 

struct _Edge
{
    void* data;
    GraphNode firstNode;
    GraphNode secondNode;
    bool isInGraph;
};

struct _Graph
{
    GraphNode  head;
    GraphCompareFunc compare;
};

static bool findNode(GraphNode root,  GraphNode node, LinkedList exploredNodes)
{
    if(root == node)
	return true;

    if(ListFind(exploredNodes, root))
	return false;

    ListInsert(exploredNodes, root);

    ListIterReset(root->edges);
    GraphEdge nextEdge = NULL;
   
    while((nextEdge = (GraphEdge) ListGetNext(root->edges)) != NULL)
    {
	if(findNode(nextEdge->secondNode, node, exploredNodes))
	    return true;
    }

    return false;
}

static bool isNodeInGraph(Graph graph, GraphNode node)
{
    LinkedList exploredNodes = ListCreateEmpty();
    
    bool isFound = findNode(graph->head, node, exploredNodes);
    ListDestroy(exploredNodes);

    return isFound; 
}

static void destroyNodes(GraphNode* node, LinkedList exploredNodes)
{
    if(!node) return;

    if(ListFind(exploredNodes, *node))
	return;

    ListInsert(exploredNodes, *node);

    ListIterReset((*node)->edges);
    GraphEdge edge = NULL;

    while((edge = (GraphEdge) ListGetNext((*node)->edges)) != NULL)
    {
	destroyNodes(&edge->secondNode, exploredNodes);
	vPortFree(edge);
    }
    
    ListDestroy((*node)->edges);
    vPortFree(*node);
    *node = NULL;
}

static bool findNodeByData(GraphNode currNode, 
			   void* data, 
			   GraphNode* node, 
			   LinkedList explored)
{
    if(currNode->data == data)
    {
	*node = currNode;
	return true;
    }

    if(ListFind(explored, currNode))
	return false;

    ListInsert(explored, currNode);

    ListIterReset(currNode->edges);
    GraphEdge nextEdge = NULL;
   
    while((nextEdge = (GraphEdge) ListGetNext(currNode->edges)) != NULL)
    {
	if(findNodeByData(nextEdge->secondNode, data, node, explored))
	    return true;
    }

    return false;
}   




#ifndef _UT

GraphNode GraphCreateNode(void* data)
{
    GraphNode node = (GraphNode) pvPortMalloc(sizeof(struct _Node));
    node->data = data;
    node->edges = ListCreateEmpty();
    node->isInGraph = false;

    return node;
}

GraphEdge GraphCreateEdge(GraphNode firstNode, GraphNode secondNode, void* data)
{
    GraphEdge edge = (GraphEdge) pvPortMalloc(sizeof(struct _Edge));
    edge->data = data;
    edge->firstNode = firstNode;
    edge->secondNode = secondNode;
    edge->isInGraph = false;

    return edge;
}

void GraphSetEdgeData(GraphEdge edge, void* data)
{
    edge->data = data;
}

void* GraphGetEdgeData(GraphEdge edge)
{
    return edge->data;
}

void GraphSetNodeData(GraphNode node, void* data)
{
    node->data = data;
}

void* GraphGetNodeData(GraphNode node)
{
    return node->data;
}

Graph GraphCreate(GraphEdge edge, GraphCompareFunc compare)
{
    Graph graph = (Graph) pvPortMalloc(sizeof(struct _Graph));
    graph->head = edge->firstNode;  
    graph->compare = compare;
    edge->isInGraph = true;
    edge->firstNode->isInGraph = true;
    edge->secondNode->isInGraph = true;

    ListInsert(graph->head->edges, edge);
    
    return graph;
}

bool GraphInsertEdge(Graph graph, GraphEdge edge)
{
   
    if(!isNodeInGraph(graph, edge->firstNode))
	return false;

    edge->isInGraph = true;
    edge->secondNode->isInGraph = true;
    ListInsert(edge->firstNode->edges, edge);
    
    return true;
}

bool GraphSetCurrentNode(Graph graph, GraphNode node)
{
    if(!isNodeInGraph(graph, node))
	return false;

    graph->head = node; 

    return true;
}

GraphNode GraphGetCurrentNode(Graph graph)
{
    return graph->head;
}

void GraphSetCompareFunc(Graph graph, GraphCompareFunc compare)
{
    graph->compare = compare;
}

bool GraphGoToNextNode(Graph graph, void* input)
{
    GraphEdge edge = NULL;

    if(!graph->head) return false;

    ListIterReset(graph->head->edges);

    while((edge = (GraphEdge) ListGetNext(graph->head->edges)) != NULL)
    {
	if(graph->compare(edge->data, input))
	{
	    graph->head = edge->secondNode;
	    return true;
	}
	 
    }

    return false;

}

bool GraphFindNode(Graph graph, void* data, GraphNode* node)
{
    LinkedList exploredNodes = ListCreateEmpty();

    return findNodeByData(graph->head, data, node, exploredNodes); 
}

bool GraphDestroyNode(GraphNode node)
{
    if(node->isInGraph)
	return false;

    vPortFree(node);
    return true;
}

bool GraphDestroyEdge(GraphEdge edge)
{
    if(edge->isInGraph)
	return false;

    vPortFree(edge);
    return true;

}

void GraphDestroy(Graph graph)
{
    if(!graph) return;
    
    LinkedList exploredNodes = ListCreateEmpty();

    destroyNodes(&graph->head, exploredNodes);

    ListDestroy(exploredNodes);
    vPortFree(graph);
}

#endif // #ifndef _UT
