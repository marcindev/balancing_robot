#include "getPostmortemCmd.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;

const double GetPostmortemCmd::CONN_TIMEOUT = 5.0;

GetPostmortemCmd::GetPostmortemCmd(shared_ptr<Connection> conn) : GetLogsCommand(conn)
{

}

GetPostmortemCmd::GetPostmortemCmd(shared_ptr<Connection> conn, const std::vector<std::string>& _args) :
		GetLogsCommand(conn, _args)
{

}


void GetPostmortemCmd::run()
{
	if(args.empty())
	{
		cout << "GetPostmortemCmd: master/slave parameter not specified" << endl;
		return;
	}

	std::string argument(args.front());

	if(argument != "0" && argument != "1")
	{
		cout << "GetPostmortemCmd: improper master/slave parameter" << endl;
		return;
	}

	bool isMaster = (argument == "0") ? false : true;

	shared_ptr<GetPostmortemMsgReq> request(new GetPostmortemMsgReq);
	*request = INIT_GET_POSTMORTEM_MSG_REQ;
	request->isMaster = isMaster;

	connection->send(shared_ptr<BaseMessage>(new Message<GetPostmortemMsgReq>(request)));

	time_t startTime, currTime;
	time(&startTime);

	vector<shared_ptr<BaseMessage>> msgVec;
//	////////////////
//	GetLogsMsgRsp getLogsRsp = INIT_GET_LOGS_MSG_RSP;
//	getLogsRsp.totalLineNum = 30;
//	uint16_t lineNum = 1;
//	/////////////

	while(connection->isConnected())
	{
		shared_ptr<BaseMessage> msg;
		if(connection->receive(msg))
		{

			uint8_t msgId = msg->getMsgId();

			if(msgId != GET_POSTMORTEM_MSG_RSP)
			{
				cout << "GetPostmortemCmd: unrecognized msg " << hex << static_cast<int>(msgId) << endl;
				continue;
			}

			time(&currTime);
			double timeDiff = difftime(currTime, startTime);

//			if(!handleResponse(reinterpret_cast<GetPostmortemMsgRsp*>(msg.getRawPayload()))
//				|| timeDiff > CONN_TIMEOUT)
			uint8_t ctrlByte = reinterpret_cast<GetPostmortemMsgRsp*>(msg->getRawPayload())->ctrlByte;
			if(ctrlByte == EMPTY || ctrlByte == LAST || timeDiff > CONN_TIMEOUT)
				break;

			msgVec.push_back(msg);


//			//////////////
//
//
//			GetPostmortemMsgRsp* pmResponse = reinterpret_cast<GetPostmortemMsgRsp*>(msg.getRawPayload());
//
//			getLogsRsp.lineNum = lineNum;
//			getLogsRsp.logLevel = pmResponse->logLevel;
//			getLogsRsp.component = pmResponse->component;
//			getLogsRsp.timestamp = pmResponse->timestamp;
//			getLogsRsp.argsNum = pmResponse->argsNum;
//			memcpy(getLogsRsp.argTypes, pmResponse->argTypes, pmResponse->argsNum);
//			memcpy(getLogsRsp.argsBuffer, pmResponse->argsBuffer, sizeof(pmResponse->argsBuffer));
//			memcpy(getLogsRsp.strBuffer, pmResponse->strBuffer, sizeof(pmResponse->strBuffer));
//
//			handleGetLogsRsp(&getLogsRsp);
//
//			++lineNum;
//
//			////////////////


		}

	}

	if(msgVec.empty())
	{
		cout << "GetPostmortemCmd: no postmortems found" << endl;
		return;
	}

	sort(msgVec.begin(), msgVec.end(),
			[](shared_ptr<BaseMessage> msg1, shared_ptr<BaseMessage> msg2)->bool
			{return reinterpret_cast<GetPostmortemMsgRsp*>(msg1->getRawPayload())->lineNum
			> reinterpret_cast<GetPostmortemMsgRsp*>(msg2->getRawPayload())->lineNum;});

	GetLogsMsgRsp getLogsRsp = INIT_GET_LOGS_MSG_RSP;
	uint16_t lineNum = 1;

	getLogsRsp.totalLineNum = msgVec.size();

	for(auto& msg : msgVec)
	{
		GetPostmortemMsgRsp* pmResponse = reinterpret_cast<GetPostmortemMsgRsp*>(msg->getRawPayload());

		getLogsRsp.lineNum = lineNum;
		getLogsRsp.logLevel = pmResponse->logLevel;
		getLogsRsp.component = pmResponse->component;
		getLogsRsp.timestamp = pmResponse->timestamp;
		getLogsRsp.argsNum = pmResponse->argsNum;
		memcpy(getLogsRsp.argTypes, pmResponse->argTypes, pmResponse->argsNum);
		memcpy(getLogsRsp.argsBuffer, pmResponse->argsBuffer, sizeof(pmResponse->argsBuffer));
		memcpy(getLogsRsp.strBuffer, pmResponse->strBuffer, sizeof(pmResponse->strBuffer));

		handleGetLogsRsp(&getLogsRsp);

		++lineNum;
	}


	for(auto& line : vecLogs)
	{
		if(line.empty())
			continue;

		cout << line << endl;
	}

	vecLogs.clear();

}


