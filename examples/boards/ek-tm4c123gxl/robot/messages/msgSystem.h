

#ifndef MSG_SYSTEM_H
#define MSG_SYSTEM_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
	Msg_RobotTaskID = 0,
	Msg_TcpServerTaskID,
	Msg_MotorsTaskID,
	Msg_I2cTaskID,
	Msg_EncoderTaskID,
	Msg_WheelsTaskID,
	Msg_ServerSpiComTaskID,
	MSG_TcpServerHandlerID
}MsgTaskId;

typedef int16_t MsgQueueId;
#define MSG_WAIT_LONG_TIME		0xFFFF

MsgQueueId registerMsgQueue(uint16_t queueSize);
MsgQueueId registerMainMsgQueue(uint8_t taskId, uint16_t queueSize);
MsgQueueId getQueueIdFromTaskId(uint8_t taskId);
bool msgReceive(MsgQueueId receiver, void** pMsg, uint16_t waitTicks);
bool msgSend(MsgQueueId sender, MsgQueueId receiver, void** pMsg, uint16_t waitTicks);
bool msgRespond(MsgQueueId receiver, void** pMsg, uint16_t waitTicks);



#endif // MSG_SYSTEM_H
