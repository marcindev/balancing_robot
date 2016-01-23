#ifndef WHEEL_RUN_CMD_H
#define WHEEL_RUN_CMD_H

#include "../messages/messages.h"
#include <memory>

#include "command.h"

class WheelRunCmd : public Command
{
public:
	WheelRunCmd(std::shared_ptr<Connection> conn);
	WheelRunCmd(std::shared_ptr<Connection> conn, const std::vector<std::string>& _args);

protected:
	void run();

};

#endif // WHEEL_RUN_CMD_H
