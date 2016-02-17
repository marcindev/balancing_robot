#ifndef GET_POSTMORTEM_CMD_H
#define GET_POSTMORTEM_CMD_H

#include "../messages/messages.h"
#include <memory>

#include "command.h"
#include "getLogsCommand.h"

class GetPostmortemCmd : public GetLogsCommand
{
public:
	enum CtrlByte { NORMAL = 0x01, LAST = 0x02, EMPTY  = 0x04};

	GetPostmortemCmd(std::shared_ptr<Connection> conn);
	GetPostmortemCmd(std::shared_ptr<Connection> conn, const std::vector<std::string>& _args);

protected:
	void run();
	bool handleResponse(GetPostmortemMsgRsp* response);
private:

	static const double CONN_TIMEOUT;
};

#endif // GET_POSTMORTEM_CMD_H
