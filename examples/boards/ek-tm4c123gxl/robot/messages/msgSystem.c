
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "msgSystem.h"

#define QUEUE_ITEM_SIZE			4
#define QUEUES_MAX_SIZE			50
#define TASK_IDS_MAX_SIZE		50

static xQueueHandle g_queues[QUEUES_MAX_SIZE] = {0};
static MsgQueueId g_queuesByTasks[TASK_IDS_MAX_SIZE] = {0};
static bool g_isTaskQueueRegistered[TASK_IDS_MAX_SIZE] = {0};
static uint16_t g_nextQueueId = 0;

MsgQueueId registerMsgQueue(uint16_t queueSize)
{
	if(g_nextQueueId > QUEUES_MAX_SIZE)
		return -1;

	g_queues[g_nextQueueId] = xQueueCreate(queueSize, QUEUE_ITEM_SIZE);

	return g_nextQueueId++;
}

MsgQueueId registerMainMsgQueue(uint8_t taskId, uint16_t queueSize)
{
	if(g_nextQueueId > QUEUES_MAX_SIZE || taskId >= TASK_IDS_MAX_SIZE)
		return -1;

	if(g_isTaskQueueRegistered[taskId])
		return g_queuesByTasks[taskId];

	MsgQueueId newQueueId = registerMsgQueue(queueSize);
	g_queuesByTasks[taskId] = newQueueId;
	g_isTaskQueueRegistered[taskId] = true;

	return newQueueId;
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
	if(receiver < 0 || receiver >= g_nextQueueId)
		return false;

	if(xQueueReceive(g_queues[receiver], pMsg, waitTicks) == pdPASS)
		return true;

	return false;
}

bool msgSend(MsgQueueId sender, MsgQueueId receiver, void** pMsg, uint16_t waitTicks)
{
	if(receiver < 0 || receiver >= g_nextQueueId)
		return false;

	if(sender < 0 || sender >= g_nextQueueId)
		return false;

	uint8_t* msgPtr = (uint8_t*) *pMsg;
	++msgPtr;
	*msgPtr = (uint8_t) sender;

	if(xQueueSend(g_queues[receiver], pMsg, waitTicks) == pdTRUE)
		return true;

	return false;
}
