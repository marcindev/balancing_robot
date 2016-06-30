#include "updaterCmd.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <fstream>

using namespace std;


UpdaterCmd::UpdaterCmd(shared_ptr<Connection> conn) : Command(conn)
{

}

UpdaterCmd::UpdaterCmd(shared_ptr<Connection> conn, const std::vector<std::string>& _args) :
        Command(conn, _args), crc32Table(256)
{
    initCRC32Table();

    commands[Cmd::START_UPDATE_UPD_CMD] = shared_ptr<UpdaterCommand>(new StartUpdateCmd(*this));
    commands[Cmd::ERASE_MEMORY_UPD_CMD] = shared_ptr<UpdaterCommand>(new EraseMemoryCmd(*this));
    commands[Cmd::SEND_DATA_UPD_CMD] = shared_ptr<UpdaterCommand>(new SendDataCmd(*this));
    commands[Cmd::FINISH_DATA_TRANSFER_UPD_CMD] = shared_ptr<UpdaterCommand>(new FinishDataTransferCmd(*this));
    commands[Cmd::FREE_RESOURCES_UPD_CMD] = shared_ptr<UpdaterCommand>(new FreeResourcesCmd(*this));
    commands[Cmd::RESET_ROBOT_UPD_CMD] = shared_ptr<UpdaterCommand>(new ResetRobotCmd(*this));
    commands[Cmd::AFTER_UPD_CHECK_UPD_CMD] = shared_ptr<UpdaterCommand>(new AfterUpdCheckCmd(*this));
    commands[Cmd::GET_AVAIL_PARTITION_UPD_CMD] = shared_ptr<UpdaterCommand>(new GetAvailPartitionCmd(*this));
    commands[Cmd::MARK_PARTITION_AS_GOOD_UPD_CMD] = shared_ptr<UpdaterCommand>(new MarkPartitionAsGoodCmd(*this));
    commands[Cmd::FORCE_NEWEST_SW_UPD_CMD] = shared_ptr<UpdaterCommand>(new ForceNewestSwCmd(*this));

    statusHandlers[Status::UPDATER_NOT_INIT_UPD_STAT] = shared_ptr<StatusHandler>(new UpdaterNotInitHandler(*this));
    statusHandlers[Status::NOT_ENOUGH_SPACE_UPD_STAT] = shared_ptr<StatusHandler>(new NotEnoughSpaceHandler(*this));
    statusHandlers[Status::READY_FOR_UPDATE_UPD_STAT] = shared_ptr<StatusHandler>(new ReadyForUpdateHandler(*this));
    statusHandlers[Status::ERASE_OK_UPD_STAT] = shared_ptr<StatusHandler>(new EraseOkHandler(*this));
    statusHandlers[Status::ERASE_ERR_UPD_UPD_STAT] = shared_ptr<StatusHandler>(new EraseErrHandler(*this));
    statusHandlers[Status::PROGRAM_DATA_OK_UPD_STAT] = shared_ptr<StatusHandler>(new ProgramDataOkHandler(*this));
    statusHandlers[Status::PROGRAM_DATA_ERR_UPD_STAT] = shared_ptr<StatusHandler>(new ProgramDataErrHandler(*this));
    statusHandlers[Status::DATA_CHECKSUM_NOK_UPD_STAT] = shared_ptr<StatusHandler>(new DataChecksumNokHandler(*this));
    statusHandlers[Status::MISSING_PACKET_UPD_STAT] = shared_ptr<StatusHandler>(new MissingPacketHandler(*this));
    statusHandlers[Status::FINISH_SUCCESS_UPD_STAT] = shared_ptr<StatusHandler>(new FinishSuccessHandler(*this));
    statusHandlers[Status::BINARY_CRC_ERR_UPD_STAT] = shared_ptr<StatusHandler>(new BinaryCrcErrHandler(*this));
    statusHandlers[Status::FREE_RESOURCES_OK_UPD_STAT] = shared_ptr<StatusHandler>(new FreeResourcesOkHandler(*this));
    statusHandlers[Status::AVAIL_PARTITION_1_CMD_UPD_STAT] = shared_ptr<StatusHandler>(new AvailPartition1Handler(*this));
    statusHandlers[Status::AVAIL_PARTITION_2_CMD_UPD_STAT] = shared_ptr<StatusHandler>(new AvailPartition2Handler(*this));
    statusHandlers[Status::UNRECOGNIZED_CMD_UPD_STAT] = shared_ptr<StatusHandler>(new UnrecognizedCmdHandler(*this));
    statusHandlers[Status::AFTER_UPD_CHECK_OK_UPD_STAT] = shared_ptr<StatusHandler>(new AfterUpdCheckOkHandler(*this));
    statusHandlers[Status::MARK_PARTITION_AS_GOOD_UPD_STAT] = shared_ptr<StatusHandler>(new MarkPartitionAsGoodOkHandler(*this));
    statusHandlers[Status::FORCE_NEWEST_SW_OK_UPD_STAT] = shared_ptr<StatusHandler>(new ForceNewestSwOkHandler(*this));


    partitions[Partitions::PARTITION_1] = shared_ptr<Partition>(new Partition());
    partitions[Partitions::PARTITION_1]->address = 0x4000;
    partitions[Partitions::PARTITION_1]->length = 0x1E000 - 0x4000;
    partitions[Partitions::PARTITION_2] = shared_ptr<Partition>(new Partition());
    partitions[Partitions::PARTITION_2]->address = 0x1E000;
    partitions[Partitions::PARTITION_2]->length = 0x40000 - 0x1E000;

    handleOptions();
}


void UpdaterCmd::run()
{
    if(args.size() < 1 || args.size() > 2)
    {
        printHelp();
        return;
    }

    if(!isConfigRead)
        return;

    std::string argument(args[0]);

    if(argument != "0" && argument != "1")
    {
        cout << "UpdaterCmd: improper master/slave parameter" << endl;
        return;
    }

    isMaster = (argument == "0") ? false : true;

    if(isForceNewest)
    {
        commands[Cmd::FORCE_NEWEST_SW_UPD_CMD]->execute();
    }
    else
    {
        commands[Cmd::GET_AVAIL_PARTITION_UPD_CMD]->execute();
    }

    while(connection->isConnected())
    {
        shared_ptr<BaseMessage> msg;
        if(connection->receive(msg))
        {
            uint8_t msgId = msg->getMsgId();

            if(msgId == UPDATER_CMD_MSG_RSP)
            {
                if(!handleResponse(*Message<UpdaterCmdMsgRsp>(*msg).getPayload()))
                    break;
            }
            else if(msgId == UPDATER_SEND_DATA_MSG_RSP)
            {
                if(!handleResponse(*Message<UpdaterSendDataMsgRsp>(*msg).getPayload()))
                    break;
            }
            else
            {
                cout << "UpdaterCmd: unrecognized msg " << hex << static_cast<int>(msgId) << endl;
                cout << dec;
            }
        }

    }

}

void UpdaterCmd::printHelp()
{
    cout << "Usage: update <master/slave> [options]" << endl;
    cout << "Options:" << endl;
    cout << "--single_word, -sw   : sends data word by word instead of 32-word packets" << endl;
    cout << "--force_newest, -fn   : forces change active partition to the one with newest sw version" << endl;
}

void UpdaterCmd::handleOptions()
{
    for(const auto& arg : args)
    {
        if(arg[0] == '-')
        {
            if(arg == "--single_word" || arg == "-sw")
            {
                isSend32Words = false;
            }

            if(arg == "--force_newest" || arg == "-fn")
            {
                isForceNewest = true;
            }

        }
    }
}


bool UpdaterCmd::openFile()
{
    const uint32_t _128_BYTES = 128,
                   _1024_BYTES = 1024;

    string binary_name = getConfValue(ConfOption::binary_name);

    cout << "Opening file " << binary_name << endl;
    ifstream file(binary_name, ios::binary);

    if(!file)
    {
        cout << "Couldn't open the file" << endl;
        return false;
    }

    streampos begin, end, size;
    file.seekg(0, file.end);
    end = file.tellg();
    file.seekg(0, file.beg);
    begin = file.tellg();
    size = end - begin;

    cout << "Binary size: " << size << endl;
    cout << "Copying data to buffer..." << endl;

    binary.reserve(size);
    binary.assign(istreambuf_iterator<char>(file), istreambuf_iterator<char>());

    uint32_t binarySize = binary.size();


    if(isSend32Words)
    {
        totalPackets = binarySize / _1024_BYTES;
        if(binarySize % _1024_BYTES) totalPackets++;
        totalPackets *= _1024_BYTES / _128_BYTES;
    }
    else
    {
        totalPackets =  binarySize / sizeof(uint32_t);
        if(binarySize % sizeof(uint32_t)) totalPackets++;
    }

    cout << "Buffer read. Size: " << binarySize << " bytes" << endl;

    return true;
}

bool UpdaterCmd::buildBinary()
{
    string make_exec = getConfValue(ConfOption::make_exec);
    string makefile_dir = getConfValue(ConfOption::makefile_dir);
    string last_target = getConfValue(ConfOption::last_target);

    bool isCleanNeeded = false;

    cout << "Preparing linker file..." << endl;

    bool isLinkerFileChanged = false;

    if(!prepareLinkerFile(isLinkerFileChanged))
    {
        cout << "ERR: Preparing linker file failed!" << endl;
        return false;
    }

    string target;

    if(isMaster)
        target = "master";
    else
        target = "slave";

    if(target != last_target || isLinkerFileChanged)
    {
        isCleanNeeded = true;
    }

    vector<string> args;
    args.push_back("-C");
    args.push_back(makefile_dir);

    if(isCleanNeeded)
    {
        args.push_back("clean");
        if(!executeProcess(make_exec, args))
        {
            cout << "ERR: Make clean failed!" << endl;
            return false;
        }
        args.pop_back();
    }

    args.push_back(target);
    if(!executeProcess(make_exec, args))
    {
        cout << "ERR: Building binary failed!" << endl;
        return false;
    }

    setConfValue(ConfOption::last_target, target);

    copyAxfFile();

    cout << "Binary built successfully." << endl;
    return true;
}

bool UpdaterCmd::copyAxfFile()
{
    string axf_file_name = getConfValue(ConfOption::axf_file_name);

    string target, partition;

    if(isMaster)
        target = "master";
    else
        target = "slave";

    if(currentPartition == Partitions::PARTITION_1)
        partition = "partition_1";
    else
        partition = "partition_2";

    if(chdir(target.c_str()) == -1)
    {
        if(mkdir(target.c_str(), S_IRWXU) == -1)
        {
            cout << "ERR: Couldn't crate a folder: " << target << endl;
            return false;
        }

        chdir(target.c_str());
    }

    if(chdir(partition.c_str()) == -1)
    {
        if(mkdir(partition.c_str(), S_IRWXU) == -1)
        {
            cout << "ERR: Couldn't crate a folder: " << partition << endl;
            return false;
        }

        chdir(partition.c_str());
    }

    chdir("../..");

    string cp("/bin/cp");
    vector<string> args;
    args.push_back(axf_file_name);
    args.push_back(string("./") + target + "/" + partition + "/");

    if(!executeProcess(cp, args))
    {
        cout << "ERR: Couldn't copy axf file!" << endl;
        return false;
    }

    return true;
}

bool UpdaterCmd::prepareLinkerFile(bool& isFileChanged)
{
    string linker_filename = getConfValue(ConfOption::linker_filename);
    string last_partition = getConfValue(ConfOption::last_partition);

    Partitions lastPartition;

    cout << "last_partition: " << last_partition << endl;

    if(last_partition == "1")
    {
        lastPartition = Partitions::PARTITION_1;
    }
    else if(last_partition == "2")
    {
        lastPartition = Partitions::PARTITION_2;
    }
    else
    {
        cout << "ERR: Incorrect last partition from config!" << endl;
        return false;
    }

    if(currentPartition == lastPartition)
    {
        isFileChanged = false;
        return true;
    }

    ifstream ifile(linker_filename);

    if(!ifile)
    {
        cout << "ERR: Couldn't open linker file!" << endl;
        return false;
    }

    string line;
    vector<string> lines;

    while(getline(ifile, line))
    {
        lines.push_back(line);
    }

    ifile.close();

    for(auto& line : lines)
    {
        size_t pos = line.find("FLASH (rx)");

        if(pos == string::npos)
            continue;

        size_t beg, end, len;
        stringstream ss;

        ss << showbase << internal << setfill('0');
        ss << hex << setw(10) << partitions[currentPartition]->address;
        string replacement;
        ss >> replacement;
        ss.str("");
        ss.clear();

        beg = line.find("0x", pos);
        end = line.find(",", beg);
        len = end - beg;
        line.replace(beg, len, replacement);

        ss << hex << setw(10) << partitions[currentPartition]->length;
        ss >> replacement;
        ss.str("");
        ss.clear();

        beg = line.find("0x", end);
        end = line.size();

        len = end - beg;
        line.replace(beg, len, replacement);

        break;
    }

    ofstream ofile(linker_filename);

    if(!ofile)
    {
        cout << "ERR: Couldn't open linker file!" << endl;
        return false;
    }

    ostream_iterator<string> output_iterator(ofile, "\n");
    copy(lines.begin(), lines.end(), output_iterator);

    isFileChanged = true;

    string strCurrPartition;

    strCurrPartition = ((currentPartition == Partitions::PARTITION_1) ? "1" : "2");

    setConfValue(ConfOption::last_partition, strCurrPartition);

    return true;

}

uint32_t UpdaterCmd::fetchNextWord()
{
    const uint32_t WORD_SIZE = 4;
    uint32_t index = packetsCnt * WORD_SIZE;
    Word32 word;
    word.u32Word = 0;

    for(int j = 0; j != WORD_SIZE; ++j, ++index)
    {
        word.bytes[j] = binary[index];
    }

    ++packetsCnt;

    return word.u32Word;
}

void UpdaterCmd::fetchNext128Bytes(uint8_t* data)
{
    const uint32_t _128_BYTES = 128;

    uint32_t index = packetsCnt * _128_BYTES;

    char* u8Data = reinterpret_cast<char*>(data);

    for(int j = 0; j != _128_BYTES; ++j, ++index)
    {
        if(index >= binary.size())
        {
            u8Data[j] = 0xFF;
            continue;
        }
        u8Data[j] = binary[index];
    }

    ++packetsCnt;

}

void UpdaterCmd::setToPrevWord()
{
    --packetsCnt;

}

bool UpdaterCmd::resetRobot()
{
    cout << "Resetting robot..." << endl;

    commands[Cmd::RESET_ROBOT_UPD_CMD]->execute();
    this_thread::sleep_for(chrono::seconds(1));

    if(!isMaster)
    {
        cout << "Resetting connection..." << endl;

        if(connection->resetConnection())
        {
            cout << "Connection established" << endl;
        }
        else
        {
            cout << "ERR: Couldn't connect to robot!" << endl;
            return false;
        }
    }

    return true;
}


UpdaterCmd::UpdaterCommand::UpdaterCommand(UpdaterCmd& _owner) : owner(_owner)
{

}

void UpdaterCmd::UpdaterCommand::execute()
{
    shared_ptr<UpdaterCmdMsgReq> request(new UpdaterCmdMsgReq);
    *request = INIT_UPDATER_CMD_MSG_REQ;
    request->isMaster = owner.isMaster;
    request->command = commandToU8();

    owner.lastCommand = command;

    fillMessage(request);

    owner.connection->send(shared_ptr<BaseMessage>(new Message<UpdaterCmdMsgReq>(request)));
}

uint8_t UpdaterCmd::UpdaterCommand::commandToU8()
{
    return static_cast<uint8_t>(command);
}

UpdaterCmd::StartUpdateCmd::StartUpdateCmd(UpdaterCmd& _owner) :
        UpdaterCommand(_owner)
{
    command = Cmd::START_UPDATE_UPD_CMD;
}

void UpdaterCmd::StartUpdateCmd::fillMessage(std::shared_ptr<UpdaterCmdMsgReq> request)
{
    uint32_t binarySize = owner.binary.size();
    request->data1 =  binarySize;
    request->data2 = owner.calculateCRC32(reinterpret_cast<uint8_t*>(owner.binary.data()),
                                          binarySize,
                                          owner.CRC_DIVISOR);

    uint32_t dataForChecksum[2] = {request->data1, request->data2};
    uint8_t checksum = owner.calcChecksum(reinterpret_cast<uint8_t*>(dataForChecksum),
                                          sizeof(dataForChecksum));

    request->data3 = checksum;

    cout << "Checksum2: " << static_cast<int>(checksum) << endl;
}

UpdaterCmd::EraseMemoryCmd::EraseMemoryCmd(UpdaterCmd& _owner) :
        UpdaterCommand(_owner)
{
    command = Cmd::ERASE_MEMORY_UPD_CMD;
}

void UpdaterCmd::EraseMemoryCmd::fillMessage(std::shared_ptr<UpdaterCmdMsgReq> request)
{

}

UpdaterCmd::SendDataCmd::SendDataCmd(UpdaterCmd& _owner) :
        UpdaterCommand(_owner)
{
    command = Cmd::SEND_DATA_UPD_CMD;
}

void UpdaterCmd::SendDataCmd::fillMessage(std::shared_ptr<UpdaterCmdMsgReq> request)
{
    request->data1 = owner.fetchNextWord();
    request->data2 = owner.calcChecksum(reinterpret_cast<uint8_t*>(&request->data1),sizeof(uint32_t));
    request->data3 = owner.packetsCnt - 1;
}

void UpdaterCmd::SendDataCmd::execute()
{
    if(owner.isSend32Words)
    {
        shared_ptr<UpdaterSendDataMsgReq> request(new UpdaterSendDataMsgReq);
        *request = INIT_UPDATER_SEND_DATA_MSG_REQ;
        request->isMaster = owner.isMaster;
        owner.fetchNext128Bytes(request->data);
        request->checksum = owner.calcChecksum(reinterpret_cast<uint8_t*>(request->data),128);
        request->partNum = owner.packetsCnt - 1;
        owner.lastCommand = command;

        owner.connection->send(shared_ptr<BaseMessage>(new Message<UpdaterSendDataMsgReq>(request)));
    }
    else
    {
        UpdaterCommand::execute();
    }

}

UpdaterCmd::FinishDataTransferCmd::FinishDataTransferCmd(UpdaterCmd& _owner) :
        UpdaterCommand(_owner)
{
    command = Cmd::FINISH_DATA_TRANSFER_UPD_CMD;
}


void UpdaterCmd::FinishDataTransferCmd::fillMessage(std::shared_ptr<UpdaterCmdMsgReq> request)
{

}

UpdaterCmd::FreeResourcesCmd::FreeResourcesCmd(UpdaterCmd& _owner) :
        UpdaterCommand(_owner)
{
    command = Cmd::FREE_RESOURCES_UPD_CMD;
}


void UpdaterCmd::FreeResourcesCmd::fillMessage(std::shared_ptr<UpdaterCmdMsgReq> request)
{

}

UpdaterCmd::ResetRobotCmd::ResetRobotCmd(UpdaterCmd& _owner) :
        UpdaterCommand(_owner)
{
    command = Cmd::RESET_ROBOT_UPD_CMD;
}


void UpdaterCmd::ResetRobotCmd::fillMessage(std::shared_ptr<UpdaterCmdMsgReq> request)
{

}

UpdaterCmd::AfterUpdCheckCmd::AfterUpdCheckCmd(UpdaterCmd& _owner) :
        UpdaterCommand(_owner)
{
    command = Cmd::AFTER_UPD_CHECK_UPD_CMD;
}

void UpdaterCmd::AfterUpdCheckCmd::fillMessage(std::shared_ptr<UpdaterCmdMsgReq> request)
{

}

UpdaterCmd::MarkPartitionAsGoodCmd::MarkPartitionAsGoodCmd(UpdaterCmd& _owner) :
        UpdaterCommand(_owner)
{
    command = Cmd::MARK_PARTITION_AS_GOOD_UPD_CMD;
}

void UpdaterCmd::MarkPartitionAsGoodCmd::fillMessage(std::shared_ptr<UpdaterCmdMsgReq> request)
{

}

UpdaterCmd::GetAvailPartitionCmd::GetAvailPartitionCmd(UpdaterCmd& _owner) :
        UpdaterCommand(_owner)
{
    command = Cmd::GET_AVAIL_PARTITION_UPD_CMD;
}

void UpdaterCmd::GetAvailPartitionCmd::fillMessage(std::shared_ptr<UpdaterCmdMsgReq> request)
{

}

UpdaterCmd::ForceNewestSwCmd::ForceNewestSwCmd(UpdaterCmd& _owner) :
        UpdaterCommand(_owner)
{
    command = Cmd::FORCE_NEWEST_SW_UPD_CMD;
}

void UpdaterCmd::ForceNewestSwCmd::fillMessage(std::shared_ptr<UpdaterCmdMsgReq> request)
{

}

void UpdaterCmd::UpdaterNotInitHandler::execute()
{
    cout << "ERR: Updater has not been initialized!" << endl;
    cout << "Releasing resources..." << endl;

    owner.commands[Cmd::FREE_RESOURCES_UPD_CMD]->execute();
    owner.isGoOn = false;
}

void UpdaterCmd::NotEnoughSpaceHandler::execute()
{
    cout << "ERR: Not enough space on flash partition!" << endl;
    cout << "Releasing resources..." << endl;

    owner.commands[Cmd::FREE_RESOURCES_UPD_CMD]->execute();
    owner.isGoOn = false;
}

void UpdaterCmd::ReadyForUpdateHandler::execute()
{
    cout << "Updater initiated" << endl;
    cout << "Erasing flash partition..." << endl;
    owner.commands[Cmd::ERASE_MEMORY_UPD_CMD]->execute();
}

void UpdaterCmd::EraseOkHandler::execute()
{
    cout << "Partition erased successfully" << endl;
    cout << "Programming binary on flash..." << endl;
    owner.commands[Cmd::SEND_DATA_UPD_CMD]->execute();
}

void UpdaterCmd::EraseErrHandler::execute()
{
    cout << "ERR: Couldn't erase flash partition!" << endl;
    cout << "Releasing resources..." << endl;

    owner.commands[Cmd::FREE_RESOURCES_UPD_CMD]->execute();
}

void UpdaterCmd::ProgramDataOkHandler::execute()
{
    cout << "Progress: " << fixed << setprecision(3) << setw(5)
         << (owner.packetsCnt / static_cast<double>(owner.totalPackets)) * 100.0 << "%\r";

    if(owner.packetsCnt == owner.totalPackets)
    {
        cout << endl;
        cout << "Finishing update..." << endl;
        owner.commands[Cmd::FINISH_DATA_TRANSFER_UPD_CMD]->execute();
    }
    else
    {
        owner.commands[Cmd::SEND_DATA_UPD_CMD]->execute();
    }

}

void UpdaterCmd::ProgramDataErrHandler::execute()
{
    cout << "ERR: Failed to program data!" << endl;
    cout << "Releasing resources..." << endl;

    owner.commands[Cmd::FREE_RESOURCES_UPD_CMD]->execute();
}

void UpdaterCmd::DataChecksumNokHandler::execute()
{
    cout << "ERR: Checksum doesn't match!" << endl;

    if(owner.lastCommand == Cmd::SEND_DATA_UPD_CMD)
    {
        cout << "Resending packet." << endl;
        owner.setToPrevWord();
        owner.commands[Cmd::SEND_DATA_UPD_CMD]->execute();
    }
    else if(owner.lastCommand == Cmd::START_UPDATE_UPD_CMD)
    {
        cout << "Retrying to start update..." << endl;
        owner.commands[Cmd::START_UPDATE_UPD_CMD]->execute();
    }
    else
    {
        cout << "Releasing resources..." << endl;

        owner.commands[Cmd::FREE_RESOURCES_UPD_CMD]->execute();
    }
}

void UpdaterCmd::MissingPacketHandler::execute()
{
    cout << "WRN: Missing packet! Resending packet." << endl;

    owner.setToPrevWord();
    owner.commands[Cmd::SEND_DATA_UPD_CMD]->execute();
}

void UpdaterCmd::FinishSuccessHandler::execute()
{
    cout << "Binary programmed successfully." << endl;
    cout << "Releasing resources..." << endl;

    owner.isAwaitingReset = true;
    owner.commands[Cmd::FREE_RESOURCES_UPD_CMD]->execute();
}

void UpdaterCmd::BinaryCrcErrHandler::execute()
{
    cout << "WRN: Programmed binary has incorrect CRC!" << endl;
    cout << "Releasing resources..." << endl;

    owner.commands[Cmd::FREE_RESOURCES_UPD_CMD]->execute();
}

void UpdaterCmd::FreeResourcesOkHandler::execute()
{
    cout << "Resources released." << endl;

    if(owner.isAwaitingReset)
    {
        owner.isAwaitingReset = false;

        if(!owner.resetRobot())
        {
            owner.isGoOn = false;
            return;
        }

        cout << "Performing after update check..." << endl;

        owner.commands[Cmd::AFTER_UPD_CHECK_UPD_CMD]->execute();
    }
    else
    {
        owner.isGoOn = false;
    }
}

void UpdaterCmd::AvailPartition1Handler::execute()
{
    cout << "Binary will be programmed on partition 1:" << endl;
    cout << "\tFlash address:" "0x" << hex << owner.partitions[Partitions::PARTITION_1]->address << endl;
    cout << "\tLength:" << "0x" << hex << owner.partitions[Partitions::PARTITION_1]->length << dec << endl;
    cout << dec;

    owner.currentPartition = Partitions::PARTITION_1;

    cout << "Building binary..." << endl;

    if(!owner.buildBinary())
    {
        owner.commands[Cmd::FREE_RESOURCES_UPD_CMD]->execute();
        return;
    }

    cout << "Opening binary ..." << endl;

    if(!owner.openFile())
    {
        owner.commands[Cmd::FREE_RESOURCES_UPD_CMD]->execute();
        return;
    }

    owner.commands[Cmd::START_UPDATE_UPD_CMD]->execute();
}

void UpdaterCmd::AvailPartition2Handler::execute()
{
    cout << "Binary will be programmed on partition 2:" << endl;
    cout << "\tFlash address:" "0x" << hex << owner.partitions[Partitions::PARTITION_2]->address << endl;
    cout << "\tLength:" << "0x" << hex << owner.partitions[Partitions::PARTITION_2]->length << dec << endl;
    cout << dec;

    owner.currentPartition = Partitions::PARTITION_2;

    cout << "Building binary..." << endl;

    if(!owner.buildBinary())
    {
        owner.commands[Cmd::FREE_RESOURCES_UPD_CMD]->execute();
        return;
    }

    cout << "Opening binary ..." << endl;

    if(!owner.openFile())
    {
        owner.commands[Cmd::FREE_RESOURCES_UPD_CMD]->execute();
        return;
    }

    owner.commands[Cmd::START_UPDATE_UPD_CMD]->execute();
}

void UpdaterCmd::AfterUpdCheckOkHandler::execute()
{
    cout << "After update check successful." << endl;
    cout << "Marking partition as good..." << endl;

    owner.commands[Cmd::MARK_PARTITION_AS_GOOD_UPD_CMD]->execute();
}


void UpdaterCmd::MarkPartitionAsGoodOkHandler::execute()
{
    cout << "Partition marked as good successfully." << endl;
    cout << "Update finished successfully!" << endl;

    owner.resetRobot();

    owner.isGoOn = false;
}

void UpdaterCmd::ForceNewestSwOkHandler::execute()
{
    cout << "Force newest software successful." << endl;
    owner.resetRobot();
    owner.isGoOn = false;
}


void UpdaterCmd::UnrecognizedCmdHandler::execute()
{
    cout << "WRN: Robot received unrecognized command!" << endl;
    owner.isGoOn = false;
}

uint8_t UpdaterCmd::calcChecksum(uint8_t* data, uint32_t size)
{
    uint32_t checksum = 0;

    while(size--)
    {
        checksum += *data++;
    }

    return checksum & 0xFF;
}

uint32_t UpdaterCmd::reflect(uint32_t ui32Ref, uint8_t ui8Ch)
{
    uint_fast32_t ui32Value;
    int_fast16_t i16Loop;

    ui32Value = 0;

    for(i16Loop = 1; i16Loop < (ui8Ch + 1); i16Loop++)
    {
        if(ui32Ref & 1)
        {
            ui32Value |= 1 << (ui8Ch - i16Loop);
        }
        ui32Ref >>= 1;
    }

    return ui32Value;
}

void UpdaterCmd::initCRC32Table()
{
    uint_fast32_t ui32Polynomial;
    int_fast16_t i16Loop, i16Bit;

    //
    // This is the ANSI X 3.66 polynomial as required by the DFU
    // specification.
    //
    ui32Polynomial = 0x04c11db7;

    for(i16Loop = 0; i16Loop <= 0xFF; i16Loop++)
    {
        crc32Table[i16Loop]=reflect(i16Loop, 8) << 24;
          for (i16Bit = 0; i16Bit < 8; i16Bit++)
          {
              crc32Table[i16Loop] = ((crc32Table[i16Loop] << 1) ^
                                            (crc32Table[i16Loop] &
                                             ((uint32_t)1 << 31) ?
                                             ui32Polynomial : 0));
          }
          crc32Table[i16Loop] = reflect(crc32Table[i16Loop], 32);
    }
}


uint32_t UpdaterCmd::calculateCRC32(uint8_t *data, uint32_t size, uint32_t ui32CRC)
{
    uint32_t ui32Count;
    uint8_t *pui8Buffer;
    uint8_t ui8Char;

    pui8Buffer = data;
    ui32Count = size;

    while(ui32Count--)
    {
        ui8Char = *pui8Buffer++;
        ui32CRC = (ui32CRC >> 8) ^ crc32Table[(ui32CRC & 0xFF) ^
                  ui8Char];
    }

    return ui32CRC;
}

UpdaterCmd::Status UpdaterCmd::u8ToStatus(uint8_t u8Status)
{
    return static_cast<Status>(u8Status);
}


