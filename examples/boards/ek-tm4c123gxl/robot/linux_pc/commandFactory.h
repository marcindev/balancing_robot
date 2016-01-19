#ifndef COMMAND_FACTORY_H
#define COMMAND_FACTORY_H

#include <iostream>
#include <memory>
#include <vector>
#include "command.h"
#include "connection.h"

class CommandFactory
{
public:
	CommandFactory(std::shared_ptr<Connection> conn);

	std::shared_ptr<Command> createCommand(const std::string& strCommand,
			const std::vector<std::string>& args);

private:
	std::shared_ptr<Connection> connection;
};


#endif // COMMAND_FACTORY_H
