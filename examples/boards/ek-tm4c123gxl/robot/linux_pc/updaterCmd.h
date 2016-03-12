#ifndef UPDATER_CMD_H
#define UPDATER_CMD_H

#include "../messages/messages.h"
#include <memory>
#include <vector>
#include <map>

#include "command.h"

union Word32
{
	uint32_t u32Word;
	uint8_t bytes[4];
};

class UpdaterCmd : public Command
{
	friend class UpdaterCommand;
	friend class StatusHandler;
public:
	enum class Cmd
	{
		START_UPDATE_UPD_CMD            = 0x01,
		ERASE_MEMORY_UPD_CMD			= 0x02,
		SEND_DATA_UPD_CMD				= 0x03,
		FINISH_DATA_TRANSFER_UPD_CMD	= 0x04,
		FREE_RESOURCES_UPD_CMD			= 0x05,
		RESET_ROBOT_UPD_CMD				= 0x06,
		AFTER_UPD_CHECK_UPD_CMD			= 0x07,
		GET_AVAIL_PARTITION_UPD_CMD	    = 0x08,
		MARK_PARTITION_AS_GOOD_UPD_CMD	= 0x09,
		FORCE_NEWEST_SW_UPD_CMD			= 0x0A
	};

	enum class Status
	{
		UPDATER_NOT_INIT_UPD_STAT		= 0x01,
		NOT_ENOUGH_SPACE_UPD_STAT		= 0x02,
		READY_FOR_UPDATE_UPD_STAT		= 0x03,
		ERASE_OK_UPD_STAT				= 0x04,
		ERASE_ERR_UPD_UPD_STAT			= 0x05,
		PROGRAM_DATA_OK_UPD_STAT		= 0x06,
		PROGRAM_DATA_ERR_UPD_STAT		= 0x07,
		DATA_CHECKSUM_NOK_UPD_STAT		= 0x08,
		MISSING_PACKET_UPD_STAT			= 0x09,
		FINISH_SUCCESS_UPD_STAT			= 0x0A,
		BINARY_CRC_ERR_UPD_STAT			= 0x0B,
		FREE_RESOURCES_OK_UPD_STAT		= 0x0C,
		UNRECOGNIZED_CMD_UPD_STAT		= 0x0D,
		AVAIL_PARTITION_1_CMD_UPD_STAT	= 0x0E,
		AVAIL_PARTITION_2_CMD_UPD_STAT	= 0x0F,
		AFTER_UPD_CHECK_OK_UPD_STAT		= 0x10,
		MARK_PARTITION_AS_GOOD_UPD_STAT = 0x11,
		FORCE_NEWEST_SW_OK_UPD_STAT 	= 0x12
	};

	enum class Partitions
	{
		PARTITION_1,
		PARTITION_2
	};

	struct Partition
	{
		uint32_t address;
		uint32_t length;
	};



	UpdaterCmd(std::shared_ptr<Connection> conn);
	UpdaterCmd(std::shared_ptr<Connection> conn, const std::vector<std::string>& _args);

protected:
	void run();
private:

	template <typename T>
	bool handleResponse(const T& response)
	{
		Status status = u8ToStatus(response.status);
		statusHandlers[status]->execute();

		return isGoOn;
	}

	void printHelp();
	void handleOptions();
	uint8_t calcChecksum(uint8_t* data, uint32_t size);
	uint32_t reflect(uint32_t ui32Ref, uint8_t ui8Ch);
	void initCRC32Table();
	uint32_t calculateCRC32(uint8_t *data, uint32_t size, uint32_t ui32CRC);
	bool openFile();
	Status u8ToStatus(uint8_t u8Status);
	uint32_t fetchNextWord();
	void fetchNext128Bytes(uint8_t* data);
	void setToPrevWord();
	bool buildBinary();
	bool prepareLinkerFile(bool& isFileChanged);
	bool copyAxfFile();
	bool resetRobot();

	class StatusHandler;
	class UpdaterCommand;
	std::map<Status, std::shared_ptr<StatusHandler>> statusHandlers;
	std::map<Cmd, std::shared_ptr<UpdaterCommand>> commands;
	std::map<Partitions, std::shared_ptr<Partition>> partitions;
	std::vector<char> binary;
	std::vector<uint32_t> crc32Table;
	const uint32_t CRC_DIVISOR = 0x09;
	bool isMaster = false;
	bool isGoOn = true;
	uint32_t packetsCnt = 0;
	uint32_t totalPackets = 0;
	bool isAwaitingReset = false;
	bool isSend32Words = true;
	bool isForceNewest = false;
	Partitions currentPartition = Partitions::PARTITION_1;
	Cmd lastCommand = Cmd::GET_AVAIL_PARTITION_UPD_CMD;

	class UpdaterCommand
	{
	public:
		UpdaterCommand(UpdaterCmd& _owner);
		virtual ~UpdaterCommand() { }
		virtual void execute();
	protected:
		virtual void fillMessage(std::shared_ptr<UpdaterCmdMsgReq> request) = 0;
		Cmd command = Cmd::START_UPDATE_UPD_CMD;
		UpdaterCmd& owner;
	private:
		uint8_t commandToU8();
	};

	class StartUpdateCmd : public UpdaterCommand
	{
	public:
		StartUpdateCmd(UpdaterCmd& _owner);
	protected:
		virtual void fillMessage(std::shared_ptr<UpdaterCmdMsgReq> request);
	};

	class EraseMemoryCmd : public UpdaterCommand
	{
	public:
		EraseMemoryCmd(UpdaterCmd& _owner);
	protected:
		virtual void fillMessage(std::shared_ptr<UpdaterCmdMsgReq> request);
	};

	class SendDataCmd : public UpdaterCommand
	{
	public:
		SendDataCmd(UpdaterCmd& _owner);
		void execute();
	protected:
		virtual void fillMessage(std::shared_ptr<UpdaterCmdMsgReq> request);
	};

	class FinishDataTransferCmd : public UpdaterCommand
	{
	public:
		FinishDataTransferCmd(UpdaterCmd& _owner);
	protected:
		virtual void fillMessage(std::shared_ptr<UpdaterCmdMsgReq> request);
	};

	class FreeResourcesCmd : public UpdaterCommand
	{
	public:
		FreeResourcesCmd(UpdaterCmd& _owner);
	protected:
		virtual void fillMessage(std::shared_ptr<UpdaterCmdMsgReq> request);
	};

	class ResetRobotCmd : public UpdaterCommand
	{
	public:
		ResetRobotCmd(UpdaterCmd& _owner);
	protected:
		virtual void fillMessage(std::shared_ptr<UpdaterCmdMsgReq> request);
	};

	class AfterUpdCheckCmd : public UpdaterCommand
	{
	public:
		AfterUpdCheckCmd(UpdaterCmd& _owner);
	protected:
		virtual void fillMessage(std::shared_ptr<UpdaterCmdMsgReq> request);
	};

	class MarkPartitionAsGoodCmd : public UpdaterCommand
	{
	public:
		MarkPartitionAsGoodCmd(UpdaterCmd& _owner);
	protected:
		virtual void fillMessage(std::shared_ptr<UpdaterCmdMsgReq> request);
	};

	class GetAvailPartitionCmd : public UpdaterCommand
	{
	public:
		GetAvailPartitionCmd(UpdaterCmd& _owner);
	protected:
		virtual void fillMessage(std::shared_ptr<UpdaterCmdMsgReq> request);
	};

	class ForceNewestSwCmd : public UpdaterCommand
	{
	public:
		ForceNewestSwCmd(UpdaterCmd& _owner);
	protected:
		virtual void fillMessage(std::shared_ptr<UpdaterCmdMsgReq> request);
	};

	class StatusHandler
	{
	public:
		StatusHandler(UpdaterCmd& _owner) : owner(_owner) { }
		virtual ~StatusHandler() { }
		virtual void execute() = 0;
	protected:
		UpdaterCmd& owner;
	};

	class UpdaterNotInitHandler : public StatusHandler
	{
	public:
		UpdaterNotInitHandler(UpdaterCmd& _owner) : StatusHandler(_owner) { }
		virtual void execute();
	};

	class NotEnoughSpaceHandler : public StatusHandler
	{
	public:
		NotEnoughSpaceHandler(UpdaterCmd& _owner) : StatusHandler(_owner) { }
		virtual void execute();
	};

	class ReadyForUpdateHandler : public StatusHandler
	{
	public:
		ReadyForUpdateHandler(UpdaterCmd& _owner) : StatusHandler(_owner) { }
		virtual void execute();
	};

	class EraseOkHandler : public StatusHandler
	{
	public:
		EraseOkHandler(UpdaterCmd& _owner) : StatusHandler(_owner) { }
		virtual void execute();
	};

	class EraseErrHandler : public StatusHandler
	{
	public:
		EraseErrHandler(UpdaterCmd& _owner) : StatusHandler(_owner) { }
		virtual void execute();
	};

	class ProgramDataOkHandler : public StatusHandler
	{
	public:
		ProgramDataOkHandler(UpdaterCmd& _owner) : StatusHandler(_owner) { }
		virtual void execute();
	};

	class ProgramDataErrHandler : public StatusHandler
	{
	public:
		ProgramDataErrHandler(UpdaterCmd& _owner) : StatusHandler(_owner) { }
		virtual void execute();
	};

	class DataChecksumNokHandler : public StatusHandler
	{
	public:
		DataChecksumNokHandler(UpdaterCmd& _owner) : StatusHandler(_owner) { }
		virtual void execute();
	};

	class MissingPacketHandler : public StatusHandler
	{
	public:
		MissingPacketHandler(UpdaterCmd& _owner) : StatusHandler(_owner) { }
		virtual void execute();
	};

	class FinishSuccessHandler : public StatusHandler
	{
	public:
		FinishSuccessHandler(UpdaterCmd& _owner) : StatusHandler(_owner) { }
		virtual void execute();
	};

	class BinaryCrcErrHandler : public StatusHandler
	{
	public:
		BinaryCrcErrHandler(UpdaterCmd& _owner) : StatusHandler(_owner) { }
		virtual void execute();
	};

	class FreeResourcesOkHandler : public StatusHandler
	{
	public:
		FreeResourcesOkHandler(UpdaterCmd& _owner) : StatusHandler(_owner) { }
		virtual void execute();
	};

	class AvailPartition1Handler : public StatusHandler
	{
	public:
		AvailPartition1Handler(UpdaterCmd& _owner) : StatusHandler(_owner) { }
		virtual void execute();
	};

	class AvailPartition2Handler : public StatusHandler
	{
	public:
		AvailPartition2Handler(UpdaterCmd& _owner) : StatusHandler(_owner) { }
		virtual void execute();
	};

	class MarkPartitionAsGoodOkHandler : public StatusHandler
	{
	public:
		MarkPartitionAsGoodOkHandler(UpdaterCmd& _owner) : StatusHandler(_owner) { }
		virtual void execute();
	};

	class AfterUpdCheckOkHandler : public StatusHandler
	{
	public:
		AfterUpdCheckOkHandler(UpdaterCmd& _owner) : StatusHandler(_owner) { }
		virtual void execute();
	};

	class ForceNewestSwOkHandler : public StatusHandler
	{
	public:
		ForceNewestSwOkHandler(UpdaterCmd& _owner) : StatusHandler(_owner) { }
		virtual void execute();
	};

	class UnrecognizedCmdHandler : public StatusHandler
	{
	public:
		UnrecognizedCmdHandler(UpdaterCmd& _owner) : StatusHandler(_owner) { }
		virtual void execute();
	};
};

#endif // UPDATER_CMD_H
