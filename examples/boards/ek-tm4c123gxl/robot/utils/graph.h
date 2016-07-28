#ifndef GRAPH_H
#define GRAPH_H

#include <stdint.h>
#include <stdbool.h>

// Connected directed graph

typedef struct _Node* GraphNode;
typedef struct _Edge* GraphEdge;
typedef struct _Graph* Graph;
typedef bool(*GraphCompareFunc)(void*, void*);

GraphNode GraphCreateNode(void* data);
GraphEdge GraphCreateEdge(GraphNode firstNode, GraphNode secondNode, void* data);
void GraphSetEdgeData(GraphEdge edge, void* data);
void* GraphGetEdgeData(GraphEdge edge);
void GraphSetNodeData(GraphNode node, void* data);
void* GraphGetNodeData(GraphNode node);
Graph GraphCreate(GraphEdge edge, GraphCompareFunc compare);
bool GraphInsertEdge(Graph graph, GraphEdge edge);
bool GraphSetCurrentNode(Graph graph, GraphNode node);
GraphNode GraphGetCurrentNode(Graph graph);
void GraphSetCompareFunc(Graph graph, GraphCompareFunc compare);
bool GraphGoToNextNode(Graph graph, void* input);
bool GraphFindNode(Graph graph, void* data, GraphNode* node);
void GraphDestroy(Graph graph);
bool GraphDestroyNode(GraphNode node);
bool GraphDestroyEdge(GraphEdge edge);



#endif // GRAPH_H
