
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "msgSystem.h"

#define QUEUE_ITEM_SIZE			4
#define QUEUES_MAX_SIZE			20
#define TASK_IDS_MAX_SIZE		20

#define SENDER_OFFSET			2

static xQueueHandle g_queues[QUEUES_MAX_SIZE] = {0};
static MsgQueueId g_queuesByTasks[TASK_IDS_MAX_SIZE] = {0};
static bool g_isTaskQueueRegistered[TASK_IDS_MAX_SIZE] = {0};

MsgQueueId registerMsgQueue(uint16_t queueSize)
{
	for(MsgQueueId queueId = 0; queueId != QUEUES_MAX_SIZE; queueId++)
	{
		if(g_queues[queueId])
		{
			g_queues[queueId] = xQueueCreate(queueSize, QUEUE_ITEM_SIZE);
			return queueId;
		}
	}

	return -1;
}

bool deregisterMsgQueue(MsgQueueId queueId)
{
	if(queueId > QUEUES_MAX_SIZE || queueId <= 0)
		return false;

	if(g_queues[queueId] == NULL)
		return false;

	vQueueDelete(g_queues[queueId]);
	g_queues[queueId] = 0;

	return true;
}

MsgQueueId registerMainMsgQueue(uint8_t taskId, uint16_t queueSize)
{
	if(taskId >= TASK_IDS_MAX_SIZE)
		return -1;

	if(g_isTaskQueueRegistered[taskId])
		return g_queuesByTasks[taskId];

	MsgQueueId newQueueId = registerMsgQueue(queueSize);
	g_queuesByTasks[taskId] = newQueueId;
	g_isTaskQueueRegistered[taskId] = true;

	return newQueueId;
}

MsgQueueId deregisterMainMsgQueue(uint8_t taskId)
{
	if(taskId >= TASK_IDS_MAX_SIZE)
		return false;

	if(!g_isTaskQueueRegistered[taskId])
		return false;

	MsgQueueId queueId = getQueueIdFromTaskId(taskId);

	deregisterMsgQueue(queueId);
	g_queuesByTasks[taskId] = 0;
	g_isTaskQueueRegistered[taskId] = false;

	return true;
}

MsgQueueId getQueueIdFromTaskId(uint8_t taskId)
{
	if(taskId >= TASK_IDS_MAX_SIZE)
		return -1;

	if(!g_isTaskQueueRegistered[taskId])
		return -1;

	return g_queuesByTasks[taskId];
}

bool msgReceive(MsgQueueId receiver, void** pMsg, uint16_t waitTicks)
{
	if(receiver < 0 || receiver >= QUEUES_MAX_SIZE)
		return false;

	if(g_queues[receiver] == NULL)
		return false;

	if(xQueueReceive(g_queues[receiver], pMsg, waitTicks) == pdPASS)
		return true;

	return false;
}

bool msgSend(MsgQueueId sender, MsgQueueId receiver, void** pMsg, uint16_t waitTicks)
{
	if(receiver < 0 || receiver >= QUEUES_MAX_SIZE)
		return false;

	if(sender < 0 || sender >= QUEUES_MAX_SIZE)
		return false;

	if(g_queues[receiver] == NULL || g_queues[sender] == NULL)
		return false;

	uint8_t* msgPtr = (uint8_t*) *pMsg;
	msgPtr += SENDER_OFFSET;
	*msgPtr = (uint8_t) sender;

	if(xQueueSend(g_queues[receiver], pMsg, waitTicks) == pdTRUE)
		return true;

	return false;
}

bool msgRespond(MsgQueueId receiver, void** pMsg, uint16_t waitTicks)
{
	if(receiver < 0 || receiver >= QUEUES_MAX_SIZE)
		return false;

	if(g_queues[receiver] == NULL)
		return false;

	if(xQueueSend(g_queues[receiver], pMsg, waitTicks) == pdTRUE)
		return true;

	return false;

}
