#include "wdg.h"
#include "driverlib/watchdog.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "timers.h"
#include "logger.h"

#define WDG_PERIOD                   5
#define WDG_TASK_NUM_MAX             20
#define WDG_MONITOR_TIMER_PERIOD     1000


typedef struct
{
    TaskHandle_t xTask;
    WdgTaskState state;
}WdgTaskStatus;

static WdgTaskStatus g_wdgTaskStatuses[WDG_TASK_NUM_MAX];
static uint8_t g_index = 0;
static bool g_isDumpPM = true;

static  TimerHandle_t g_wdgTimer;
static uint32_t g_wdgCounter;

static void initWdgMonitorTimer();
static void wdgMonitorTimerCallback(TimerHandle_t pxTimer);

void initWatchDog()
{
    initInterrupts();
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
    IntEnable(INT_WATCHDOG);
    WatchdogReloadSet(WATCHDOG0_BASE, WDG_PERIOD * SysCtlClockGet());
    WatchdogStallEnable(WATCHDOG0_BASE);
    WatchdogResetEnable(WATCHDOG0_BASE);
    WatchdogEnable(WATCHDOG0_BASE);

    initWdgMonitorTimer();
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
    g_wdgTaskStatuses[g_index].state = WDG_UNKNOWN;
    return (int16_t) g_index++;
}

void feedWatchDog(uint8_t taskId, WdgTaskState state)
{
    if(taskId >= WDG_TASK_NUM_MAX)
        return;

    g_wdgTaskStatuses[taskId].state = state;

}

void onWatchDogTimeout()
{
//  const char* strStates[] = {"Running", "Ready", "Blocked", "Suspended", "Deleted"};
//  const char* strPrefix = "[onWatchDogTimeout] Task: ";
//  const char* strState = ", state: ";
//  const char* strIsFed = ", isFed: ";

    logger(Error, Log_Robot, "[onWatchDogTimeout] watchdog timed out");

//  size_t strPrefixLen = strlen(strPrefix);
//  char strBuffer[800]; // TODO: may overwrite memory if pui32Stack is not increased
//
//  char* strBuffPtr = strBuffer;
//
//  for(uint8_t i = 0; i != g_index; ++i)
//  {
//      char* prevStrBuffPtr = strBuffPtr;
//      memcpy(strBuffPtr, strPrefix, strPrefixLen);
//      strBuffPtr += strPrefixLen;
//
//      size_t strLen = 0;
//      char* strTaskName = pcTaskGetTaskName(g_wdgTaskStatuses[i].xTask);
//      strLen = strlen(strTaskName);
//      memcpy(strBuffPtr, strTaskName, strLen);
//      strBuffPtr += strLen;
//      strLen = strlen(strState);
//      memcpy(strBuffPtr, strState, strLen);
//      strBuffPtr += strLen;
//      uint8_t taskState = (uint8_t) eTaskGetState(g_wdgTaskStatuses[i].xTask);
//      strLen = strlen(strStates[taskState]);
//      memcpy(strBuffPtr, strStates[taskState], strLen);
//      strBuffPtr += strLen;
//      strLen = strlen(strIsFed);
//      memcpy(strBuffPtr, strIsFed, strLen);
//      strBuffPtr += strLen;
//      const char* strTrueFalse = g_wdgTaskStatuses[i].isFed ? "true" : "false";
//      strLen = strlen(strTrueFalse);
//      memcpy(strBuffPtr, strTrueFalse, strLen);
//      strBuffPtr += strLen;
//      *strBuffPtr = '\0';
//      ++strBuffPtr;
//
//      logger(Info, Log_Robot, prevStrBuffPtr);
//  }Log_Leds

    logStackTrace();

    if(g_isDumpPM && !g_wdgCounter)
        dumpPostMortem();

    g_wdgCounter++;

}

void initWdgMonitorTimer()
{

    g_wdgTimer = xTimerCreate(
            "WdgMonitorTimer",
            pdMS_TO_TICKS(WDG_MONITOR_TIMER_PERIOD),
            pdTRUE,
            ( void * ) 0,
            wdgMonitorTimerCallback
    );

    if(g_wdgTimer == NULL)
    {
        logger(Error, Log_Robot, "[initWdgMonitorTimer] Couldn't create timer");
        return;
    }

    logger(Debug, Log_Robot, "[initWdgMonitorTimer] timer created");

    if( xTimerStart( g_wdgTimer, 0 ) != pdPASS )
    {
        logger(Error, Log_Robot, "[initWdgMonitorTimer] Couldn't start timer");
    }

    logger(Debug, Log_Robot, "[initWdgMonitorTimer] timer started");
}

void wdgMonitorTimerCallback(TimerHandle_t pxTimer)
{
    for(uint8_t i = 0; i != g_index; ++i)
    {
        if(g_wdgTaskStatuses[i].state == WDG_UNKNOWN)
            return;
    }

    for(uint8_t i = 0; i != g_index; ++i)
    {
        if(g_wdgTaskStatuses[i].state == WDG_ALIVE)
            g_wdgTaskStatuses[i].state = WDG_UNKNOWN;
    }

    WatchdogReloadSet(WATCHDOG0_BASE, WDG_PERIOD * SysCtlClockGet());

}


