#include "getTaskListCmd.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;

GetTaskListCmd::Part::Part() : id(0)
{

}

GetTaskListCmd::GetTaskListCmd(shared_ptr<Connection> conn) : Command(conn)
{

}

GetTaskListCmd::GetTaskListCmd(shared_ptr<Connection> conn, const std::vector<std::string>& _args) :
		Command(conn, _args)
{

}


void GetTaskListCmd::run()
{
	if(args.empty())
	{
		cout << "GetTaskListCmd: master/slave parameter not specified" << endl;
		return;
	}

	std::string argument(args.front());

	if(argument != "0" && argument != "1")
	{
		cout << "GetTaskListCmd: improper master/slave parameter" << endl;
		return;
	}

	bool isMaster = (argument == "0") ? false : true;

	GetTaskListReq* request = new GetTaskListReq;
	*request = INIT_GET_TASK_LIST_MSG_REQ;
	request->isMaster = isMaster;
	shared_ptr<void> payload(request);

	Message msg(payload);
	connection->send(msg);

	while(connection->isConnected())
	{
		Message msg;
		if(connection->receive(msg))
		{
			uint8_t msgId = *(reinterpret_cast<uint8_t*>(msg.getRawPayload()));

			if(msgId != GET_TASK_LIST_MSG_RSP)
				cout << "GetTaskListCmd: unrecognized msg " << hex << static_cast<int>(msgId) << endl;

			if(!handleResponse(reinterpret_cast<GetTaskListRsp*>(msg.getRawPayload())))
				break;
		}

	}

}

bool GetTaskListCmd::handleResponse(GetTaskListRsp* response)
{
	size_t totalParts = response->totalParts;

	Part part;
	part.id = response->partId;
	part.str = reinterpret_cast<const char*>(response->strBuffer);
	parts.push_back(part);

	if(parts.size() == totalParts)
	{
		sort(parts.begin(), parts.end(),
				[](const Part& p1, const Part& p2){return p1.id < p2.id;});

		string report;

		for(auto& p : parts)
		{
			report += p.str;
		}

		cout << "Name          State  Priority  Stack   Num" << endl;
		cout << "------------------------------------------" << endl;
		cout << report << endl;

		return false;
	}

	return true;
}

