#ifndef GET_MCTRL_DATA_CMD_H
#define GET_MCTRL_DATA_CMD_H

#include "../messages/messages.h"
#include <memory>

#include "command.h"
#include <string>
#include <fstream>

class GetMctrlDataCmd : public Command
{
public:
    GetMctrlDataCmd(std::shared_ptr<Connection> conn);
    GetMctrlDataCmd(std::shared_ptr<Connection> conn, const std::vector<std::string>& _args);

protected:
    enum Option
    {
        OneShot,
        Periodically,
        Continuously
    };


    void run();
private:

    bool handleResponse(const MctrlGetDataMsgRsp& response);
    void printHelp();
    void handleOptions();

    Option option;
    uint32_t period;
    bool isHelp = false;
    bool isDump = false;
    std::string fileName;
    std::ofstream file;

};

#endif // GET_MCTRL_DATA_CMD_H
