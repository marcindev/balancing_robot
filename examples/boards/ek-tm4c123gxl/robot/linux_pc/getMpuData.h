#ifndef GET_MPU_DATA_CMD_H
#define GET_MPU_DATA_CMD_H

#include "../messages/messages.h"
#include <memory>

#include "command.h"

class GetMpuDataCmd : public Command
{
public:
    GetMpuDataCmd(std::shared_ptr<Connection> conn);
    GetMpuDataCmd(std::shared_ptr<Connection> conn, const std::vector<std::string>& _args);

protected:
    enum Option
    {
        OneShot,
        Periodically,
        Continuously
    };


    void run();
private:

    bool handleResponse(const MpuGetDataTcpMsgRsp& response);
    void printHelp();
    void handleOptions();

    Option option;
    uint32_t period;
    bool isHelp = false;

};

#endif // GET_MPU_DATA_CMD_H
