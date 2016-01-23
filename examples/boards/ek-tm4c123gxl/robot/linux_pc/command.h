#ifndef COMMAND_H
#define COMMAND_H

#include <iostream>
#include <thread>
#include <memory>
#include <vector>

#include "connection.h"

class Command
{
public:
	Command(std::shared_ptr<Connection> conn);
	Command(std::shared_ptr<Connection> conn, const std::vector<std::string>& _args);
	virtual ~Command() { }

	virtual void execute();
	virtual void wait();
protected:
	virtual void run() = 0;

	std::shared_ptr<Connection> connection;
	std::shared_ptr<std::thread> _thread;
	std::vector<std::string> args;

};


#endif // COMMAND_H
