#include "wdg.h"
#include "driverlib/watchdog.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "logger.h"

#define WDG_PERIOD					 5
#define WDG_TASK_NUM_MAX			 20

typedef struct
{
	TaskHandle_t xTask;
	bool isFed;
}WdgTaskStatus;

static WdgTaskStatus g_wdgTaskStatuses[WDG_TASK_NUM_MAX];
static uint8_t g_index = 0;
static bool g_isDumpPM = true;

void initWatchDog()
{
	initInterrupts();
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
	IntEnable(INT_WATCHDOG);
	WatchdogReloadSet(WATCHDOG0_BASE, WDG_PERIOD * SysCtlClockGet());
	WatchdogStallEnable(WATCHDOG0_BASE);
	WatchdogResetEnable(WATCHDOG0_BASE);
	WatchdogEnable(WATCHDOG0_BASE);
}

void setIsDumpPM(bool isOn)
{
	g_isDumpPM = isOn;
}

int16_t registerToWatchDog()
{
	if(g_index >= WDG_TASK_NUM_MAX)
		return -1;

	TaskHandle_t xTask = xTaskGetCurrentTaskHandle();
	g_wdgTaskStatuses[g_index].xTask = xTask;
	g_wdgTaskStatuses[g_index].isFed = true;
	return (int16_t) g_index++;
}

void feedWatchDog(uint8_t taskId)
{
	if(taskId >= WDG_TASK_NUM_MAX)
		return;

	g_wdgTaskStatuses[taskId].isFed = true;

	for(uint8_t i = 0; i != g_index; ++i)
	{
		if(!g_wdgTaskStatuses[i].isFed)
		{
			eTaskState taskState = eTaskGetState(g_wdgTaskStatuses[i].xTask);

			if((taskState == eRunning) || (taskState == eReady))
				return;
		}
	}

	for(uint8_t i = 0; i != g_index; ++i)
	{
		g_wdgTaskStatuses[i].isFed = false;
	}

	WatchdogReloadSet(WATCHDOG0_BASE, WDG_PERIOD * SysCtlClockGet());
}

void onWatchDogTimeout()
{
//	const char* strStates[] = {"Running", "Ready", "Blocked", "Suspended", "Deleted"};
//	const char* strPrefix = "[onWatchDogTimeout] Task: ";
//	const char* strState = ", state: ";
//	const char* strIsFed = ", isFed: ";

	logger(Error, Log_Robot, "[onWatchDogTimeout] watchdog timed out");

//	size_t strPrefixLen = strlen(strPrefix);
//	char strBuffer[800]; // TODO: may overwrite memory if pui32Stack is not increased
//
//	char* strBuffPtr = strBuffer;
//
//	for(uint8_t i = 0; i != g_index; ++i)
//	{
//		char* prevStrBuffPtr = strBuffPtr;
//		memcpy(strBuffPtr, strPrefix, strPrefixLen);
//		strBuffPtr += strPrefixLen;
//
//		size_t strLen = 0;
//		char* strTaskName = pcTaskGetTaskName(g_wdgTaskStatuses[i].xTask);
//		strLen = strlen(strTaskName);
//		memcpy(strBuffPtr, strTaskName, strLen);
//		strBuffPtr += strLen;
//		strLen = strlen(strState);
//		memcpy(strBuffPtr, strState, strLen);
//		strBuffPtr += strLen;
//		uint8_t taskState = (uint8_t) eTaskGetState(g_wdgTaskStatuses[i].xTask);
//		strLen = strlen(strStates[taskState]);
//		memcpy(strBuffPtr, strStates[taskState], strLen);
//		strBuffPtr += strLen;
//		strLen = strlen(strIsFed);
//		memcpy(strBuffPtr, strIsFed, strLen);
//		strBuffPtr += strLen;
//		const char* strTrueFalse = g_wdgTaskStatuses[i].isFed ? "true" : "false";
//		strLen = strlen(strTrueFalse);
//		memcpy(strBuffPtr, strTrueFalse, strLen);
//		strBuffPtr += strLen;
//		*strBuffPtr = '\0';
//		++strBuffPtr;
//
//		logger(Info, Log_Robot, prevStrBuffPtr);
//	}

	logStackTrace();

	if(g_isDumpPM)
		dumpPostMortem();

}


