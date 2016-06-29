#ifndef SET_MCTRL_PERIOD_CMD_H
#define SET_MCTRL_PERIOD_CMD_H

#include "../messages/messages.h"
#include <memory>

#include "command.h"

class SetMctrlPeriodCmd : public Command
{
public:
	SetMctrlPeriodCmd(std::shared_ptr<Connection> conn);
	SetMctrlPeriodCmd(std::shared_ptr<Connection> conn, const std::vector<std::string>& _args);

protected:
	void run();
private:

	bool handleResponse(const MctrlSetPeriodMsgRsp& response);

};

#endif // SET_MCTRL_PERIOD_CMD_H
