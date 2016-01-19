#include "command.h"

using namespace std;

Command::Command(std::shared_ptr<Connection> conn) : connection(conn)
{

}

Command::Command(std::shared_ptr<Connection> conn, const std::vector<std::string>& _args) :
		connection(conn),
		args(_args)
{

}


void Command::execute()
{
	_thread.reset(new thread(&Command::run, this));
}


void Command::wait()
{
	if(!_thread)
		return;

	_thread->join();

}
