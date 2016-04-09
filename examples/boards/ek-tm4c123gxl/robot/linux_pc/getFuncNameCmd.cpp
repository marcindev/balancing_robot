#include "getFuncNameCmd.h"
#include "stackEncoder.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;

GetFuncNameCmd::GetFuncNameCmd(shared_ptr<Connection> conn) : Command(conn)
{

}

GetFuncNameCmd::GetFuncNameCmd(shared_ptr<Connection> conn, const std::vector<std::string>& _args) :
		Command(conn, _args)
{

}


void GetFuncNameCmd::run()
{
	if(args.empty())
	{
		cout << "Usage: getFuncNameCmd <isMaster> <partition> <address>" << endl;
		return;
	}

	if(args.size() != 3)
	{
		cout << "Incorrect number of parameters required 3" << endl;
		return;
	}

	std::string argument(args[0]);

	if(argument != "0" && argument != "1")
	{
		cout << "GetFuncNameCmd: improper master/slave parameter" << endl;
		return;
	}

	bool isMaster = (argument == "0") ? false : true;

	argument = args[1];

	if(argument != "1" && argument != "2")
	{
		cout << "GetFuncNameCmd: improper master/slave parameter" << endl;
		return;
	}

	uint8_t partition = (argument == "1") ? 1 : 2;

	uint32_t address;
	stringstream ss;


	ss << hex << args[2];
	ss >> address;

	if(!ss.good())
	{
		cout << "Incorrect argument: \"" << args[2] << "\"" << endl;
		return;
	}

	StackEncoder addrEncoder(isMaster, partition);

	string funcName = addrEncoder.getFuncName(address);

	if(funcName.empty())
	{
		cout << "Couldn't find a function with a given address" << endl;
		return;
	}

	cout << "Address: " << "0x" << hex << address << ", function: " << funcName << endl;
	cout << dec;

}
