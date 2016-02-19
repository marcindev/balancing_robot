#include "getFreeHeapSizeCommand.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;

GetFreeHeapSizeCommand::GetFreeHeapSizeCommand(shared_ptr<Connection> conn) : Command(conn)
{

}

GetFreeHeapSizeCommand::GetFreeHeapSizeCommand(shared_ptr<Connection> conn, const std::vector<std::string>& _args) :
		Command(conn, _args)
{

}


void GetFreeHeapSizeCommand::run()
{
	if(args.empty())
	{
		cout << "GetFreeHeapSizeCommand: master/slave parameter not specified" << endl;
		return;
	}

	std::string argument(args.front());

	if(argument != "0" && argument != "1")
	{
		cout << "GetFreeHeapSizeCommand: improper master/slave parameter" << endl;
		return;
	}

	bool isMaster = (argument == "0") ? false : true;

	shared_ptr<GetFreeHeapSizeReq> request(new GetFreeHeapSizeReq);
	*request = INIT_GET_FREE_HEAP_SIZE_MSG_REQ;
	request->isMaster = isMaster;

	connection->send(shared_ptr<BaseMessage>(new Message<GetFreeHeapSizeReq>(request)));

	while(connection->isConnected())
	{
		shared_ptr<BaseMessage> msg;
		if(connection->receive(msg))
		{
			uint8_t msgId = msg->getMsgId();

			if(msgId != GET_FREE_HEAP_SIZE_MSG_RSP)
			{
				cout << "GetFreeHeapSizeCommand: unrecognized msg " << hex << static_cast<int>(msgId) << endl;
			}
			else
			{
				handleResponse(reinterpret_cast<GetFreeHeapSizeRsp*>(msg->getRawPayload()));
				return;
			}
		}

	}

}

void GetFreeHeapSizeCommand::handleResponse(GetFreeHeapSizeRsp* response)
{
	cout << "Free heap size: " << response->heapSize << " words." << endl;
}

