#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include "getMctrlDataCmd.h"

using namespace std;

GetMctrlDataCmd::GetMctrlDataCmd(shared_ptr<Connection> conn) :
		Command(conn),
		option(OneShot),
		period(0)

{

}

GetMctrlDataCmd::GetMctrlDataCmd(shared_ptr<Connection> conn, const std::vector<std::string>& _args) :
		Command(conn, _args),
		option(OneShot),
		period(0)
{
	handleOptions();

	if(isDump)
		file.open(fileName);
}


void GetMctrlDataCmd::run()
{
	if(args.empty())
	{
		printHelp();
		return;
	}

	if(isHelp)
	{
		printHelp();
		return;
	}


	stringstream ss;
	uint32_t param;

	ss << args[0];
	ss >> param;

	if(ss.fail())
	{
		cout << "Incorrect argument: \"" << args[0] << "\"" << endl;
		return;
	}

	ss.str("");
	ss.clear();


	shared_ptr<MctrlGetDataMsgReq> request(new MctrlGetDataMsgReq);
	*request = INIT_MCTRL_GET_DATA_MSG_REQ;

	request->param = static_cast<uint8_t>(param);

	connection->send(shared_ptr<BaseMessage>(new Message<MctrlGetDataMsgReq>(request)));

	while(connection->isConnected())
	{
		shared_ptr<BaseMessage> msg;
		if(connection->receive(msg))
		{

			uint8_t msgId = msg->getMsgId();

			if(msgId != MCTRL_GET_DATA_MSG_RSP)
			{
				cout << "getMpuDataCmd: unrecognized msg " << hex << static_cast<int>(msgId) << endl;
				continue;
			}

			handleResponse(*Message<MctrlGetDataMsgRsp>(*msg).getPayload());

			switch(option)
			{
			case OneShot:
				return;
			case Continuously:
				break;
			case Periodically:
				this_thread::sleep_for(chrono::milliseconds(period));
				break;
			}

			connection->send(shared_ptr<BaseMessage>(new Message<MctrlGetDataMsgReq>(request)));

		}

	}

}

void GetMctrlDataCmd::printHelp()
{
	cout << "Usage: getMctrlData <dataType> [options]" << endl;
	cout << "dataType - {0 - inclination, 1 - pidOutput}" << endl;
	cout << "Options:" << endl;
	cout << "--help, -h	 : prints help" << endl;
	cout << "--continuous, -c   : receives data continuously" << endl;
	cout << "--periodic, -p <period>  : receives data periodically, period in milliseconds must be specified" << endl;
	cout << "--dump, -d <filename>  : dumps data to file" << endl;

}

void GetMctrlDataCmd::handleOptions()
{
	size_t cnt = 0;

	for(const auto& arg : args)
	{
		++cnt;

		if(arg[0] == '-')
		{


			if(arg == "--help" || arg == "-h")
			{
				isHelp = true;
				break;
			}

			if(arg == "--continuous" || arg == "-c")
			{
				option = Continuously;
			}

			if(arg == "--periodic" || arg == "-p")
			{
				option = Periodically;

				if(cnt >= args.size())
				{
					isHelp = true;
				}
				else
				{
					stringstream ss;

					ss << args[cnt];
					ss >> period;

					if(ss.fail())
					{
						cout << "Incorrect argument: \"" << args[cnt] << "\"" << endl;

						isHelp = true;
						return;
					}

				}

			}

			if(arg == "--dump" || arg == "-d")
			{
				isDump = true;

				if(cnt >= args.size())
				{
					isHelp = true;
				}
				else
				{
					fileName = args[cnt];
				}
			}

		}



	}
}

bool GetMctrlDataCmd::handleResponse(const MctrlGetDataMsgRsp& response)
{
	if(!response.status)
	{
		cout << "getMctrlDataCmd: couldn't read the register" << endl;
	}
	else
	{

		if(isDump)
		{
			file << fixed << setprecision(3);
			file << response.data << endl;
		}
		else
		{
			cout << fixed << setprecision(3);
			cout << "data: " << response.data << "           \r";

			cout.flush();
		}
	}



	return false;
}

