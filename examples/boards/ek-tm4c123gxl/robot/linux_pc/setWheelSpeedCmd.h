#ifndef SET_WHEEL_SPEED_CMD_H
#define SET_WHEEL_SPEED_CMD_H

#include "../messages/messages.h"
#include <memory>

#include "command.h"

class SetWheelSpeedCmd : public Command
{
public:
	SetWheelSpeedCmd(std::shared_ptr<Connection> conn);
	SetWheelSpeedCmd(std::shared_ptr<Connection> conn, const std::vector<std::string>& _args);

protected:
	void run();

};

#endif // SET_WHEEL_SPEED_CMD_H
