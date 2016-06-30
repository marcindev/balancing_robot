#ifndef READ_MPU_REG_CMD_H
#define READ_MPU_REG_CMD_H

#include "../messages/messages.h"
#include <memory>

#include "command.h"

class ReadMpuRegCmd : public Command
{
public:
    ReadMpuRegCmd(std::shared_ptr<Connection> conn);
    ReadMpuRegCmd(std::shared_ptr<Connection> conn, const std::vector<std::string>& _args);

protected:
    void run();
private:

    bool handleResponse(const MpuRegReadMsgRsp& response);

};

#endif // READ_MPU_REG_CMD_H
