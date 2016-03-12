#ifndef GET_FUNC_NAME_CMD_H
#define GET_FUNC_NAME_CMD_H

#include "../messages/messages.h"
#include <memory>
#include <iostream>
#include <sstream>

#include "command.h"

class GetFuncNameCmd : public Command
{
public:
	GetFuncNameCmd(std::shared_ptr<Connection> conn);
	GetFuncNameCmd(std::shared_ptr<Connection> conn, const std::vector<std::string>& _args);

protected:
	void run();


};

#endif // GET_FUNC_NAME_CMD_H
