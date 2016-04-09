#ifndef WRITE_MPU_REG_CMD_H
#define WRITE_MPU_REG_CMD_H

#include "../messages/messages.h"
#include <memory>

#include "command.h"

class WriteMpuRegCmd : public Command
{
public:
	WriteMpuRegCmd(std::shared_ptr<Connection> conn);
	WriteMpuRegCmd(std::shared_ptr<Connection> conn, const std::vector<std::string>& _args);

protected:
	void run();
private:

	bool handleResponse(const MpuRegWriteMsgRsp& response);

};

#endif // WRITE_MPU_REG_CMD_H
