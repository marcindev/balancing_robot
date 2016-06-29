#ifndef SET_PID_DIRECTION_CMD_H
#define SET_PID_DIRECTION_CMD_H

#include "../messages/messages.h"
#include <memory>

#include "command.h"

class SetPidDirectionCmd : public Command
{
public:
	SetPidDirectionCmd(std::shared_ptr<Connection> conn);
	SetPidDirectionCmd(std::shared_ptr<Connection> conn, const std::vector<std::string>& _args);

protected:
	void run();
private:

	bool handleResponse(const MctrlSetPidDirMsgRsp& response);

};

#endif // SET_PID_DIRECTION_CMD_H
