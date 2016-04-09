#include "readMpuRegCmd.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;

ReadMpuRegCmd::ReadMpuRegCmd(shared_ptr<Connection> conn) : Command(conn)
{

}

ReadMpuRegCmd::ReadMpuRegCmd(shared_ptr<Connection> conn, const std::vector<std::string>& _args) :
		Command(conn, _args)
{

}


void ReadMpuRegCmd::run()
{
	if(args.empty())
	{
		cout << "Usage: readMpuReg <regAddres>" << endl;
		return;
	}

	stringstream ss;
	uint32_t mpuReg;

	ss <<  hex << args[0];
	ss >> mpuReg;

	if(ss.fail())
	{
		cout << "Incorrect argument: \"" << args[0] << "\"" << endl;
		return;
	}

	ss.str("");
	ss.clear();

	cout << "Reading register 0x" << hex << mpuReg << endl;
	cout << dec;

	shared_ptr<MpuRegReadMsgReq> request(new MpuRegReadMsgReq);
	*request = INIT_MPU_REG_READ_MSG_REQ;
	request->regAddr = static_cast<uint8_t>(mpuReg);

	connection->send(shared_ptr<BaseMessage>(new Message<MpuRegReadMsgReq>(request)));

	while(connection->isConnected())
	{
		shared_ptr<BaseMessage> msg;
		if(connection->receive(msg))
		{

			uint8_t msgId = msg->getMsgId();

			if(msgId != MPU_REG_READ_MSG_RSP)
			{
				cout << "ReadMpuRegCmd: unrecognized msg " << hex << static_cast<int>(msgId) << endl;
				continue;
			}

			if(!handleResponse(*Message<MpuRegReadMsgRsp>(*msg).getPayload()))
				break;
		}

	}

}

bool ReadMpuRegCmd::handleResponse(const MpuRegReadMsgRsp& response)
{
	if(!response.status)
	{
		cout << "ReadMpuRegCmd: couldn't read the register" << endl;
	}
	else
	{
		cout << "Register value: 0x" << hex << static_cast<unsigned>(response.regVal) << endl;
		cout << dec;
	}

	return false;
}

