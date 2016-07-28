#include "gtest/gtest.h"
#include <stdlib.h>
#include "gmock/gmock.h"
//#ifndef _UT
//#define _UT
//#endif
extern "C" {
#include "graph.c"
}
#include "linkedListMock.h"
#include "testFixture.h"
#include "helper.h"
#include <vector>

using ::testing::_;
using ::testing::Return;
using ::testing::InSequence;
using ::testing::Invoke;

namespace RobotTest {

class GraphTest : public TestFixture, public Helper
{
public:
    GraphTest()
    {
	ON_CALL(*_rtosMock, pvPortMalloc(_))
	    .WillByDefault(Invoke(this, &GraphTest::mallocWrapper));
	ON_CALL(*_rtosMock, vPortFree(_))
	    .WillByDefault(Invoke(this, &GraphTest::freeWrapper));

   
    }

    ~GraphTest()
    {
    
    }


};



template <>
GraphNode Helper::allocNextObject<GraphNode>()
{
    GraphNode node = static_cast<GraphNode>(malloc(sizeof(struct _Node)));
    objectPtrs.push_back(node); 

    return node;
}

template <>
GraphEdge Helper::allocNextObject<GraphEdge>()
{
    GraphEdge edge = static_cast<GraphEdge>(malloc(sizeof(struct _Edge)));
    objectPtrs.push_back(edge); 

    return edge;
}

template <>
Graph Helper::allocNextObject<Graph>()
{
    Graph graph = static_cast<Graph>(malloc(sizeof(struct _Graph)));
    objectPtrs.push_back(graph); 

    return graph;
}



static bool someCompare(void* data1, void* data2) { return true;}

static bool compareInts(void* data1, void* data2)
{
    return *static_cast<int*>(data1) == *static_cast<int*>(data2);
}

TEST_F(GraphTest, GraphCreateNode_create_success)
{
    {
	InSequence seq;
	EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	    .WillOnce(InvokeWithoutArgs(this, &GraphTest::allocNextObject<GraphNode>));

    }

    int data = 123;

    GraphNode node = GraphCreateNode(&data);
	      
    EXPECT_EQ(123, *(static_cast<int*>(node->data)));
}

TEST_F(GraphTest, GraphCreateEdge_create_success)
{
    {
	InSequence seq;
	EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	    .WillOnce(InvokeWithoutArgs(this, &GraphTest::allocNextObject<GraphEdge>));

    }

    int data = 123;

    GraphNode fNode, sNode;

    GraphEdge edge = GraphCreateEdge(fNode, sNode, &data);

    EXPECT_EQ(123, *(static_cast<int*>(edge->data)));

}


TEST_F(GraphTest, GraphSetEdgeData_setData_dataSetCorrectly)
{
    EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	.WillOnce(InvokeWithoutArgs(this, &GraphTest::allocNextObject<GraphEdge>));

    
    int data = 123;
    GraphNode fNode, sNode;
    GraphEdge edge = GraphCreateEdge(fNode, sNode, &data);

    EXPECT_EQ(123, *(static_cast<int*>(edge->data)));

    int data2 = 456;
    GraphSetEdgeData(edge, &data2); 


    EXPECT_EQ(456, *(static_cast<int*>(edge->data)));
}


TEST_F(GraphTest, GraphGetEdgeData_getData_correctData)
{
    EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	.WillOnce(InvokeWithoutArgs(this, &GraphTest::allocNextObject<GraphEdge>));

    
    int data = 123;
    GraphNode fNode, sNode;
    GraphEdge edge = GraphCreateEdge(fNode, sNode, &data);

    EXPECT_EQ(123, *(static_cast<int*>(GraphGetEdgeData(edge))));

    int data2 = 456;
    GraphSetEdgeData(edge, &data2); 


    EXPECT_EQ(456, *(static_cast<int*>(GraphGetEdgeData(edge))));
}

TEST_F(GraphTest, GraphSetNodeData_setData_dataSetCorrectly)
{
    EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	.WillOnce(InvokeWithoutArgs(this, &GraphTest::allocNextObject<GraphNode>));

    
    int data = 123;
    GraphNode node = GraphCreateNode(&data);

    EXPECT_EQ(123, *(static_cast<int*>(node->data)));

    int data2 = 456;
    GraphSetNodeData(node, &data2); 


    EXPECT_EQ(456, *(static_cast<int*>(node->data)));
}


TEST_F(GraphTest, GraphGetNodeData_getData_correctData)
{
    EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	.WillOnce(InvokeWithoutArgs(this, &GraphTest::allocNextObject<GraphNode>));

    
    int data = 123;
    GraphNode node = GraphCreateNode(&data);

    EXPECT_EQ(123, *(static_cast<int*>(GraphGetNodeData(node))));

    int data2 = 456;
    GraphSetNodeData(node, &data2); 


    EXPECT_EQ(456, *(static_cast<int*>(GraphGetNodeData(node))));
}

TEST_F(GraphTest, GraphCreate_graphCreated_success)
{
    {
	InSequence seq;
	EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	    .Times(2)
	    .WillRepeatedly(InvokeWithoutArgs(this, &GraphTest::allocNextObject<GraphNode>));
	
	EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	    .WillOnce(InvokeWithoutArgs(this, &GraphTest::allocNextObject<GraphEdge>));

	EXPECT_CALL(*_rtosMock, pvPortMalloc(_))
	    .WillOnce(InvokeWithoutArgs(this, &GraphTest::allocNextObject<Graph>));
    }

    EXPECT_CALL(*_linkedListMock, ListInsert(_,_)).Times(1);

    int data = 123;

    GraphNode fNode = GraphCreateNode(&data),
	      sNode = GraphCreateNode(&data);
    GraphEdge edge = GraphCreateEdge(fNode, sNode, &data);

    Graph graph = GraphCreate(edge, someCompare);


}

TEST_F(GraphTest, GraphInsertEdge_firstNodeIsNotInGraph_fails)
{
    int data = 123;

    _linkedListMock->disable();

    GraphNode fNode = GraphCreateNode(&data),
	      sNode = GraphCreateNode(&data),
	      fNode2 = GraphCreateNode(&data),
	      sNode2 = GraphCreateNode(&data);
	    
    GraphEdge edge = GraphCreateEdge(fNode, sNode, &data),
	      edge2 = GraphCreateEdge(fNode2, sNode2, &data);

    Graph graph = GraphCreate(edge, someCompare);

    EXPECT_FALSE(GraphInsertEdge(graph, edge2));

    _linkedListMock->enable();
}

TEST_F(GraphTest, GraphInsertEdge_firstNodeIsInGraph_succeeds)
{
    int data = 123;

    _linkedListMock->disable();

    GraphNode fNode = GraphCreateNode(&data),
	      sNode = GraphCreateNode(&data),
	      sNode2 = GraphCreateNode(&data);
	    
    GraphEdge edge = GraphCreateEdge(fNode, sNode, &data),
	      edge2 = GraphCreateEdge(sNode, sNode2, &data);

    Graph graph = GraphCreate(edge, someCompare);

    EXPECT_TRUE(GraphInsertEdge(graph, edge2));

    _linkedListMock->enable();
}

TEST_F(GraphTest, GraphInsertEdge_firstNodeIsNotThanIsInGraph_firstFailsThanSucceeds)
{
    int data = 123;

    _linkedListMock->disable();

    GraphNode fNode = GraphCreateNode(&data),
	      sNode = GraphCreateNode(&data),
	      fNode2 = GraphCreateNode(&data),
	      sNode2 = GraphCreateNode(&data),
	      sNode3 = GraphCreateNode(&data);
	    
    GraphEdge edge = GraphCreateEdge(fNode, sNode, &data),
	      edge2 = GraphCreateEdge(fNode2, sNode2, &data),
	      edge3 = GraphCreateEdge(sNode, sNode3, &data);

    Graph graph = GraphCreate(edge, someCompare);

    EXPECT_FALSE(GraphInsertEdge(graph, edge2));
    EXPECT_TRUE(GraphInsertEdge(graph, edge3));

    _linkedListMock->enable();
}


TEST_F(GraphTest, GraphGetCurrentNode_getNodeAfterGraphCreation_returnsFirstNodeOfFirstEdge)
{
    int data = 123;

    _linkedListMock->disable();

    GraphNode fNode = GraphCreateNode(&data),
	      sNode = GraphCreateNode(&data);

	    
    GraphEdge edge = GraphCreateEdge(fNode, sNode, &data);
    Graph graph = GraphCreate(edge, someCompare);

    GraphNode resNode = GraphGetCurrentNode(graph);

    EXPECT_EQ(fNode, resNode);

    _linkedListMock->enable();
}


TEST_F(GraphTest, GraphGetCurrentNode_getNodeAfterEdgeInsertion_returnsFirstNodeOfFirstEdge)
{
    int data = 123;

    _linkedListMock->disable();

    GraphNode fNode = GraphCreateNode(&data),
	      sNode = GraphCreateNode(&data),
	      sNode2 = GraphCreateNode(&data);
	    
    GraphEdge edge = GraphCreateEdge(fNode, sNode, &data),
	      edge2 = GraphCreateEdge(sNode, sNode2, &data);

    Graph graph = GraphCreate(edge, someCompare);

    EXPECT_TRUE(GraphInsertEdge(graph, edge2));

    GraphNode resNode = GraphGetCurrentNode(graph);

    EXPECT_EQ(fNode, resNode);

    _linkedListMock->enable();
}


TEST_F(GraphTest, GraphSetCurrentNode_nodeIsNotInGraph_fails)
{
    int data = 123;

    _linkedListMock->disable();

    GraphNode fNode = GraphCreateNode(&data),
	      sNode = GraphCreateNode(&data),
	      node2 = GraphCreateNode(&data);
	    
    GraphEdge edge = GraphCreateEdge(fNode, sNode, &data);

    Graph graph = GraphCreate(edge, someCompare);

    EXPECT_FALSE(GraphSetCurrentNode(graph, node2));

    GraphNode resNode = GraphGetCurrentNode(graph);

    EXPECT_EQ(fNode, resNode);

    _linkedListMock->enable();
}

TEST_F(GraphTest, GraphSetCurrentNode_nodeIsInGraphAfterGraphCreation_succeeds)
{
    int data = 123;

    _linkedListMock->disable();

    GraphNode fNode = GraphCreateNode(&data),
	      sNode = GraphCreateNode(&data);
	    
    GraphEdge edge = GraphCreateEdge(fNode, sNode, &data);

    Graph graph = GraphCreate(edge, someCompare);
    GraphNode resNode = GraphGetCurrentNode(graph);

    EXPECT_EQ(fNode, resNode);
    EXPECT_TRUE(GraphSetCurrentNode(graph, sNode));

    resNode = GraphGetCurrentNode(graph);

    EXPECT_EQ(sNode, resNode);

    _linkedListMock->enable();
}


TEST_F(GraphTest, GraphSetCurrentNode_nodeIsInGraphAfterEdgeInsertion_succeeds)
{
    int data = 123;

    _linkedListMock->disable();

    GraphNode fNode = GraphCreateNode(&data),
	      sNode = GraphCreateNode(&data),
	      sNode2 = GraphCreateNode(&data);
	    
    GraphEdge edge = GraphCreateEdge(fNode, sNode, &data),
	      edge2 = GraphCreateEdge(sNode, sNode2, &data);

    Graph graph = GraphCreate(edge, someCompare);
    GraphNode resNode = GraphGetCurrentNode(graph);

    EXPECT_EQ(fNode, resNode);
    EXPECT_TRUE(GraphInsertEdge(graph, edge2));
    EXPECT_TRUE(GraphSetCurrentNode(graph, sNode2));

    resNode = GraphGetCurrentNode(graph);

    EXPECT_EQ(sNode2, resNode);

    _linkedListMock->enable();
}


TEST_F(GraphTest, GraphGoToNextNode_compareReturnsFalse_fails)
{
    int nodeData = 123,
	edgeData1 = 5,
	edgeData2 = 10;

    _linkedListMock->disable();

    GraphNode fNode = GraphCreateNode(&nodeData),
	      sNode = GraphCreateNode(&nodeData),
	      sNode2 = GraphCreateNode(&nodeData);
	    
    GraphEdge edge = GraphCreateEdge(fNode, sNode, &edgeData1),
	      edge2 = GraphCreateEdge(sNode, sNode2, &edgeData2);

    Graph graph = GraphCreate(edge, compareInts);

    EXPECT_TRUE(GraphInsertEdge(graph, edge2));

    GraphNode resNode = GraphGetCurrentNode(graph);

    EXPECT_EQ(fNode, resNode);

    int input = 2;

    EXPECT_FALSE(GraphGoToNextNode(graph, &input));

    resNode = GraphGetCurrentNode(graph);

    EXPECT_EQ(fNode, resNode);

    _linkedListMock->enable();
}

TEST_F(GraphTest, GraphGoToNextNode_compareReturnsTrue_succeeds)
{
    int nodeData = 123,
	edgeData1 = 5,
	edgeData2 = 10;

    _linkedListMock->disable();

    GraphNode fNode = GraphCreateNode(&nodeData),
	      sNode = GraphCreateNode(&nodeData),
	      sNode2 = GraphCreateNode(&nodeData);
	    
    GraphEdge edge = GraphCreateEdge(fNode, sNode, &edgeData1),
	      edge2 = GraphCreateEdge(sNode, sNode2, &edgeData2);

    Graph graph = GraphCreate(edge, compareInts);

    EXPECT_TRUE(GraphInsertEdge(graph, edge2));
    GraphNode resNode = GraphGetCurrentNode(graph);
    EXPECT_EQ(fNode, resNode);

    int input = 5;

    EXPECT_TRUE(GraphGoToNextNode(graph, &input));
    resNode = GraphGetCurrentNode(graph);
    EXPECT_EQ(sNode, resNode);

    input = 10;
    EXPECT_TRUE(GraphGoToNextNode(graph, &input));
    resNode = GraphGetCurrentNode(graph);
    EXPECT_EQ(sNode2, resNode);

    _linkedListMock->enable();
}

TEST_F(GraphTest, GraphGoToNextNode_cyclic_returnsToStart)
{
    int nodeData = 123,
	edgeData1 = 1,
	edgeData2 = 2,
	edgeData3 = 3,
	edgeData4 = 4;

    _linkedListMock->disable();

    GraphNode fNode = GraphCreateNode(&nodeData),
	      sNode = GraphCreateNode(&nodeData),
	      sNode2 = GraphCreateNode(&nodeData),
	      sNode3 = GraphCreateNode(&nodeData);
	    
    GraphEdge edge = GraphCreateEdge(fNode, sNode, &edgeData1),
	      edge2 = GraphCreateEdge(sNode, sNode2, &edgeData2),
	      edge3 = GraphCreateEdge(sNode2, sNode3, &edgeData3),
	      edge4 = GraphCreateEdge(sNode3, fNode, &edgeData4);

    Graph graph = GraphCreate(edge, compareInts);

    EXPECT_TRUE(GraphInsertEdge(graph, edge2));
    EXPECT_TRUE(GraphInsertEdge(graph, edge3));
    EXPECT_TRUE(GraphInsertEdge(graph, edge4));

    GraphNode resNode = GraphGetCurrentNode(graph);
    EXPECT_EQ(fNode, resNode);

    int input = 1;

    EXPECT_TRUE(GraphGoToNextNode(graph, &input));
    resNode = GraphGetCurrentNode(graph);
    EXPECT_EQ(sNode, resNode);

    input = 2;
    EXPECT_TRUE(GraphGoToNextNode(graph, &input));
    resNode = GraphGetCurrentNode(graph);
    EXPECT_EQ(sNode2, resNode);

    input = 3;
    EXPECT_TRUE(GraphGoToNextNode(graph, &input));
    resNode = GraphGetCurrentNode(graph);
    EXPECT_EQ(sNode3, resNode);

    input = 4;
    EXPECT_TRUE(GraphGoToNextNode(graph, &input));
    resNode = GraphGetCurrentNode(graph);
    EXPECT_EQ(fNode, resNode);


    _linkedListMock->enable();
}

TEST_F(GraphTest, GraphFindNode_nodeNotInGraph_fails)
{
    int nodeData1 = 1,
	nodeData2 = 2,
	nodeData3 = 3,
	nodeData4 = 4,
	nodeData5 = 5,
	edgeData1 = 1,
	edgeData2 = 2,
	edgeData3 = 3,
	edgeData4 = 4;

    _linkedListMock->disable();

    GraphNode fNode = GraphCreateNode(&nodeData1),
	      sNode = GraphCreateNode(&nodeData2),
	      sNode2 = GraphCreateNode(&nodeData3),
	      sNode3 = GraphCreateNode(&nodeData4),
	      node4 = GraphCreateNode(&nodeData5);
	    
    GraphEdge edge = GraphCreateEdge(fNode, sNode, &edgeData1),
	      edge2 = GraphCreateEdge(sNode, sNode2, &edgeData2),
	      edge3 = GraphCreateEdge(sNode2, sNode3, &edgeData3),
	      edge4 = GraphCreateEdge(sNode3, fNode, &edgeData4);

    Graph graph = GraphCreate(edge, compareInts);

    EXPECT_TRUE(GraphInsertEdge(graph, edge2));
    EXPECT_TRUE(GraphInsertEdge(graph, edge3));
    EXPECT_TRUE(GraphInsertEdge(graph, edge4));

    GraphNode resNode = 0;

    EXPECT_FALSE(GraphFindNode(graph, &nodeData5, &resNode));

    EXPECT_EQ(0, resNode);

    _linkedListMock->enable();
}

TEST_F(GraphTest, GraphFindNode_nodeWithGivenDataNotInGraph_fails)
{
    int nodeData1 = 1,
	nodeData2 = 2,
	nodeData3 = 3,
	nodeData4 = 4,
	nodeData5 = 5,
	edgeData1 = 1,
	edgeData2 = 2,
	edgeData3 = 3,
	edgeData4 = 4;

    _linkedListMock->disable();

    GraphNode fNode = GraphCreateNode(&nodeData1),
	      sNode = GraphCreateNode(&nodeData2),
	      sNode2 = GraphCreateNode(&nodeData3),
	      sNode3 = GraphCreateNode(&nodeData4);
	    
    GraphEdge edge = GraphCreateEdge(fNode, sNode, &edgeData1),
	      edge2 = GraphCreateEdge(sNode, sNode2, &edgeData2),
	      edge3 = GraphCreateEdge(sNode2, sNode3, &edgeData3),
	      edge4 = GraphCreateEdge(sNode3, fNode, &edgeData4);

    Graph graph = GraphCreate(edge, compareInts);

    EXPECT_TRUE(GraphInsertEdge(graph, edge2));
    EXPECT_TRUE(GraphInsertEdge(graph, edge3));
    EXPECT_TRUE(GraphInsertEdge(graph, edge4));

    GraphNode resNode = 0;
    
    EXPECT_FALSE(GraphFindNode(graph, &nodeData5, &resNode));

    EXPECT_EQ(0, resNode);

    _linkedListMock->enable();
}


TEST_F(GraphTest, GraphFindNode_nodeWithGivenDataIsInGraph_succeeds)
{
    int nodeData1 = 1,
	nodeData2 = 2,
	nodeData3 = 3,
	nodeData4 = 4,
	edgeData1 = 1,
	edgeData2 = 2,
	edgeData3 = 3,
	edgeData4 = 4;

    _linkedListMock->disable();

    GraphNode fNode = GraphCreateNode(&nodeData1),
	      sNode = GraphCreateNode(&nodeData2),
	      sNode2 = GraphCreateNode(&nodeData3),
	      sNode3 = GraphCreateNode(&nodeData4);
	    
    GraphEdge edge = GraphCreateEdge(fNode, sNode, &edgeData1),
	      edge2 = GraphCreateEdge(sNode, sNode2, &edgeData2),
	      edge3 = GraphCreateEdge(sNode2, sNode3, &edgeData3),
	      edge4 = GraphCreateEdge(sNode3, fNode, &edgeData4);

    Graph graph = GraphCreate(edge, compareInts);

    EXPECT_TRUE(GraphInsertEdge(graph, edge2));
    EXPECT_TRUE(GraphInsertEdge(graph, edge3));
    EXPECT_TRUE(GraphInsertEdge(graph, edge4));

    GraphNode resNode = 0;
    
    EXPECT_TRUE(GraphFindNode(graph, &nodeData3, &resNode));

    EXPECT_EQ(sNode2, resNode);

    _linkedListMock->enable();
}

TEST_F(GraphTest, GraphDestroyNode_nodeIsInGraph_fails)
{
    int data = 123;

    _linkedListMock->disable();

    GraphNode fNode = GraphCreateNode(&data),
	      sNode = GraphCreateNode(&data),
	      sNode2 = GraphCreateNode(&data);
	    
    GraphEdge edge = GraphCreateEdge(fNode, sNode, &data),
	      edge2 = GraphCreateEdge(sNode, sNode2, &data);

    Graph graph = GraphCreate(edge, someCompare);

    EXPECT_TRUE(GraphInsertEdge(graph, edge2));
    EXPECT_FALSE(GraphDestroyNode(fNode));
    EXPECT_FALSE(GraphDestroyNode(sNode));
    EXPECT_FALSE(GraphDestroyNode(sNode2));
    
    _linkedListMock->enable();
}

TEST_F(GraphTest, GraphDestroyNode_nodeIsNotInGraph_succeedsAndMemoryFreed)
{
    int data = 123;

    EXPECT_CALL(*_rtosMock, vPortFree(_)).WillOnce(Invoke(this, &GraphTest::freeWrapper));

    GraphNode node = GraphCreateNode(&data);
	    
    EXPECT_TRUE(GraphDestroyNode(node));
    EXPECT_EQ(0, getAllocCnt());
    
}


TEST_F(GraphTest, GraphDestroyEdge_edgeIsInGraph_fails)
{
    int data = 123;

    _linkedListMock->disable();

    GraphNode fNode = GraphCreateNode(&data),
	      sNode = GraphCreateNode(&data),
	      sNode2 = GraphCreateNode(&data);
	    
    GraphEdge edge = GraphCreateEdge(fNode, sNode, &data),
	      edge2 = GraphCreateEdge(sNode, sNode2, &data);

    Graph graph = GraphCreate(edge, someCompare);

    EXPECT_TRUE(GraphInsertEdge(graph, edge2));
    EXPECT_FALSE(GraphDestroyEdge(edge));
    EXPECT_FALSE(GraphDestroyEdge(edge2));
    
    _linkedListMock->enable();
}

TEST_F(GraphTest, GraphDestroyEdge_edgeIsNotInGraph_succeedsAndMemoryFreed)
{
    int data = 123;

    EXPECT_CALL(*_rtosMock, vPortFree(_))
	.Times(3)
	.WillRepeatedly(Invoke(this, &GraphTest::freeWrapper));

    GraphNode fNode = GraphCreateNode(&data),
	      sNode = GraphCreateNode(&data);
	    
    GraphEdge edge = GraphCreateEdge(fNode, sNode, &data);

    EXPECT_TRUE(GraphDestroyEdge(edge));
    EXPECT_TRUE(GraphDestroyNode(fNode));
    EXPECT_TRUE(GraphDestroyNode(sNode));
    EXPECT_EQ(0, getAllocCnt());
    
}


TEST_F(GraphTest, GraphDestroy_destroy_allAllocatedMemoryIsFreed)
{
    int nodeData1 = 1,
	nodeData2 = 2,
	nodeData3 = 3,
	nodeData4 = 4,
	edgeData1 = 1,
	edgeData2 = 2,
	edgeData3 = 3,
	edgeData4 = 4;

    _linkedListMock->disable();

    GraphNode fNode = GraphCreateNode(&nodeData1),
	      sNode = GraphCreateNode(&nodeData2),
	      sNode2 = GraphCreateNode(&nodeData3),
	      sNode3 = GraphCreateNode(&nodeData4);
    GraphEdge edge = GraphCreateEdge(fNode, sNode, &edgeData1),
	      edge2 = GraphCreateEdge(sNode, sNode2, &edgeData2),
	      edge3 = GraphCreateEdge(sNode2, sNode3, &edgeData3),
	      edge4 = GraphCreateEdge(sNode3, fNode, &edgeData4);

    Graph graph = GraphCreate(edge, compareInts);

    EXPECT_TRUE(GraphInsertEdge(graph, edge2));
    EXPECT_TRUE(GraphInsertEdge(graph, edge3));
    EXPECT_TRUE(GraphInsertEdge(graph, edge4));

    EXPECT_NE(0, getAllocCnt());

    GraphDestroy(graph);

    EXPECT_EQ(0, getAllocCnt());

    _linkedListMock->enable();
}


}
