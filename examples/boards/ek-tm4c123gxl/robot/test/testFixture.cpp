#include "testFixture.h"



std::unique_ptr<RtosMock> TestFixture::_rtosMock;
std::unique_ptr<StateMachineMock> TestFixture::_stateMachineMock;
std::unique_ptr<GraphMock> TestFixture::_graphMock;
std::unique_ptr<LinkedListMock> TestFixture::_linkedListMock;
