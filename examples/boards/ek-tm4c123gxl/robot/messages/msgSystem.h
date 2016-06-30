

#ifndef MSG_SYSTEM_H
#define MSG_SYSTEM_H

#include <stdbool.h>
#include <stdint.h>

typedef int16_t MsgQueueId;
#define MSG_WAIT_LONG_TIME      0xFFFF

typedef enum
{
    Msg_RobotTaskID = 0,
    Msg_TcpServerTaskID,
    Msg_MotorsTaskID,
    Msg_I2cTaskID,
    Msg_EncoderTaskID,
    Msg_WheelsTaskID,
    Msg_ServerSpiComTaskID,
    Msg_UpdaterTaskID,
    Msg_MpuTaskID,
    Msg_MotionControlTaskID,
    MSG_TcpServerHandlerID
}MsgTaskId;

typedef struct
{
    MsgQueueId queueId;
    uint16_t slot;
} MsgAddress;


MsgQueueId registerMsgQueue(uint16_t queueSize);
MsgQueueId registerMainMsgQueue(uint8_t taskId, uint16_t queueSize);
MsgQueueId getQueueIdFromTaskId(uint8_t taskId);
MsgAddress getAddressFromTaskId(uint8_t taskId);
bool msgReceive(MsgQueueId receiver, void** pMsg, uint16_t waitTicks);
bool msgSend(MsgQueueId sender, MsgAddress receiver, void** pMsg, uint16_t waitTicks);
bool msgRespond(MsgAddress receiver, void** pMsg, uint16_t waitTicks);
MsgAddress msgGetAddress(void* pMsg);
void msgSetAddress(void* pMsg, MsgAddress address);

#endif // MSG_SYSTEM_H
