//***********************************************
// Heart of robot functionality
// autor: Marcin Gozdziewski
//
//***********************************************

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_nvic.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/fpu.h"
#include "driverlib/eeprom.h"
#include "driverlib/watchdog.h"
#include "inc/hw_ints.h"
#include "FreeRTOS.h"
#include "portable.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "robot.h"
#include "messages.h"
#include "msgSystem.h"
#include "priorities.h"
#include "utils.h"
#include "interrupts.h"
#include "led.h"
#include "logger.h"
#include "updater/updaterTask.h"
#include "wdg.h"
#include "config.h"

#ifdef _ROBOT_MASTER_BOARD
#include "drivers/i2c/i2cTask.h"
#include "drivers/encoder.h"
#include "drivers/wheelsTask.h"
#include "drivers/MPU6050.h"
#include "drivers/motionControlComTask.h"
#include "drivers/motionControlTask.h"
#else
#include "server/tcpServerTask.h"
#endif

#include "utils/uartstdio.h"
extern void _edata;
extern void _ebss;
extern void _data;
extern void _bss;
// local globals
#ifdef _ROBOT_MASTER_BOARD
#define ROBOT_TASK_STACK_SIZE		150         // Stack size in words
#else
#define ROBOT_TASK_STACK_SIZE		100         // Stack size in words
#endif
#define ROBOT_TASK_QUEUE_SIZE		 10



//static I2cManager g_i2cManager;
//static GpioExpander g_gpioExpander;

const HeapRegion_t xHeapRegions[] =
{
	{ ( uint8_t * ) 0x20003000UL, 0x05000 },
    { NULL, 0 } /* Terminates the array. */
};

void vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName)
{
	char strLog[100];
	char* strPtr = strLog;
	const char* strPrefix = "[vApplicationStackOverflowHook] task: ";

	size_t strLen = strlen(strPrefix);
	memcpy(strPtr, strPrefix, strLen);
	strPtr += strLen;

	strLen = strlen(strPrefix);
	memcpy(strPtr, strPrefix, strLen);
	strPtr += strLen;
	*strPtr = '\0';

	logger(Error, Log_Robot, strPtr);

    while(1)
    {
    }
}

void vApplicationMallocFailedHook()
{
	logger(Error, Log_Robot, "[vApplicationMallocFailedHook] Out of memory");
	logStackTrace();
}

static MsgQueueId g_robotQueue;

static void initializeSysClock();
static void initilizeFreeRTOS();
static bool initRobotTask();
static void enableInterrupts();
static void initializeFPU();
static bool startTcpServer();
static bool startSpiServerCom();
static bool startUpdater();
static bool startWheels();
static bool startMpu();
static bool startMotionControl();
static bool startMotors();
static bool initializeEEPROM();
static void onReset();

static bool g_isWdgReset = false;

void runRobot()
{
	initializeRobot();
	ConfigureUART();
	onReset();
	UARTprintf("Starting robot\n");
#ifdef _ROBOT_MASTER_BOARD

	if(!i2cTaskInit())
	{
		while(1){ }
	}

	if(!motorsTaskInit())
	{
		while(1){ }
	}

	if(!mpuTaskInit())
	{
		while(1){ }
	}


	if(!motionControlComTaskInit())
	{
		while(1){ }
	}


//	if(!wheelsTaskInit())
//	{
//		while(1){ }
//	}

#endif

	if(!initRobotTask())
	{
		while(1){ }
	}

	if(!serverSpiComTaskInit())
	{
		while(1){ }
	}

	if(!updaterTaskInit())
	{
		while(1) { }
	}

#ifndef _ROBOT_MASTER_BOARD
	if(!tcpServerTaskInit())
	{
		while(1){ }
	}

#endif
	uint32_t beg_data = ( uint32_t ) &_data;
	uint32_t end_data = ( uint32_t ) &_edata;
	uint32_t beg_bss = ( uint32_t ) &_bss;
	uint32_t end_bss = ( uint32_t ) &_ebss;


	vTaskStartScheduler();
}

void initializeRobot()
{
	initializeFPU();
	initializeSysClock();
	if(!initializeEEPROM()) while(1){};
	enableInterrupts();
	initilizeFreeRTOS();
	initWatchDog(); // must be invoked after initilizeFreeRTOS()

}

static void robotTask(void *pvParameters)
{
	portTickType ui32WakeTime;
	ui32WakeTime = xTaskGetTickCount();

	afterWdgReset();



	if(!startSpiServerCom())
	{
		while(1){}
	}

	if(!startUpdater())
	{
		while(1) {}
	}

#ifndef _ROBOT_MASTER_BOARD

	if(!startTcpServer())
	{
		while(1){}
	}


	while(1) {
		vTaskDelayUntil(&ui32WakeTime, pdMS_TO_TICKS(2000));
	}

#else
	if(!startMotors())
	{
		while(1){}
	}

	if(!startMpu())
	{
		while(1){}
	}

	if(!startMotionControl())
	{
		while(1){}
	}

//	if(!startWheels())
//	{
//		while(1){}
//	}

//	LedInit(LED2);
//	I2cManager* i2cManager = (I2cManager*) pvPortMalloc(sizeof(I2cManager));
//	Mpu6050* mpu = (Mpu6050*) pvPortMalloc(sizeof(Mpu6050));
//	ZeroBuffer(mpu, sizeof(Mpu6050));
//	mpu->i2cManager = i2cManager;
//	MpuInit(mpu);
//
//	MpuRawData rawData;
//	ZeroBuffer(&rawData, sizeof(MpuRawData));
//
	while(1){
		vTaskDelayUntil(&ui32WakeTime, pdMS_TO_TICKS(2000));

//		MpuReadRawData(mpu, &rawData);
//
//		logger(Info, Log_Mpu, "Mpu data: %d, %d, %d, %d, %d, %d, %d",
//				rawData.acX, rawData.acY, rawData.acZ,
//				rawData.gyX, rawData.gyY, rawData.gyZ, rawData.tmp);


//		LedTurnOnOff(LED2, isWdgLedOn);

	}
/*
	I2cManager* i2cManager = (I2cManager*) pvPortMalloc(sizeof(I2cManager));
	GpioExpander* gpioExpander = (GpioExpander*) pvPortMalloc(sizeof(GpioExpander));
//
	ZeroBuffer(gpioExpander, sizeof(GpioExpander));
	gpioExpander->i2cManager = i2cManager;
	gpioExpander->hwAddress		= 0x21;
	GpioExpInit(gpioExpander);
//
	portTickType ui32WakeTime;
////	initializeGpioExpanders();
//
	ui32WakeTime = xTaskGetTickCount();
//
////	EncoderInstance encoder;
////	encoder.gpioExpander = gpioExpander;
////	encoder.ch1_pin = GPIOEXP_PIN1;
////	encoder.ch2_pin = GPIOEXP_PIN2;
////
////	initEncoder(&encoder);
//
	GpioExpSetPinDirOut(gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN7);
	GpioExpSetPinDirOut(gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN6);
//
	uint8_t direction = 0;

	WheelSetSpeedMsgReq* wheelLeftSetSpeedReq = (WheelSetSpeedMsgReq*) pvPortMalloc(sizeof(WheelSetSpeedMsgReq));
	*wheelLeftSetSpeedReq = INIT_WHEEL_SET_SPEED_MSG_REQ;
	wheelLeftSetSpeedReq->wheelId = 0;
	wheelLeftSetSpeedReq->speed = 0.5f;

	if(!msgSend(g_robotQueue, getQueueIdFromTaskId(Msg_WheelsTaskID), (void**) &wheelLeftSetSpeedReq, portMAX_DELAY))
	{
		while(1) {}
	}

	WheelSetSpeedMsgReq* wheelRightSetSpeedReq = (WheelSetSpeedMsgReq*) pvPortMalloc(sizeof(WheelSetSpeedMsgReq));
	*wheelRightSetSpeedReq = INIT_WHEEL_SET_SPEED_MSG_REQ;
	wheelRightSetSpeedReq->wheelId = 1;
	wheelRightSetSpeedReq->speed = 1.0f;

	if(!msgSend(g_robotQueue, getQueueIdFromTaskId(Msg_WheelsTaskID), (void**) &wheelRightSetSpeedReq, portMAX_DELAY))
	{
		while(1) {}
	}


//	WheelRunMsgReq* wheelLeftRunReq = (WheelRunMsgReq*) pvPortMalloc(sizeof(WheelRunMsgReq));
//	*wheelLeftRunReq = INIT_WHEEL_RUN_MSG_REQ;
//	wheelLeftRunReq->wheelId = 0;
//	wheelLeftRunReq->direction = 0;
//	wheelLeftRunReq->rotations = 2.0f;
//
//	if(!msgSend(g_robotQueue, getQueueIdFromTaskId(Msg_WheelsTaskID), (void**) &wheelLeftRunReq, portMAX_DELAY))
//	{
//		while(1) {}
//	}
//
//	WheelRunMsgReq* wheelRightRunReq = (WheelRunMsgReq*) pvPortMalloc(sizeof(WheelRunMsgReq));
//	*wheelRightRunReq = INIT_WHEEL_RUN_MSG_REQ;
//	wheelRightRunReq->wheelId = 1;
//	wheelRightRunReq->direction = 1;
//	wheelRightRunReq->rotations = 10.0f;
//
//	if(!msgSend(g_robotQueue, getQueueIdFromTaskId(Msg_WheelsTaskID), (void**) &wheelRightRunReq, portMAX_DELAY))
//	{
//		while(1) {}
//	}

//	int motorsNum = 2;
//
//	for(int i = 0; i != motorsNum; ++i)
//	{
//	MotorStartMsgReq* motorStartReq = (MotorStartMsgReq*) pvPortMalloc(sizeof(MotorStartMsgReq));
//
//	*motorStartReq = INIT_MOTOR_START_MSG_REQ;
//	motorStartReq->motorId = i;
//
//	if(!msgSend(g_robotQueue, getQueueIdFromTaskId(Msg_MotorsTaskID), (void**) &motorStartReq, portMAX_DELAY))
//	{
//		while(1) {}
//	}
//	}


	while(true)
	{
		WheelRunMsgReq* wheelLeftRunReq = (WheelRunMsgReq*) pvPortMalloc(sizeof(WheelRunMsgReq));
		*wheelLeftRunReq = INIT_WHEEL_RUN_MSG_REQ;
		wheelLeftRunReq->wheelId = 0;
		wheelLeftRunReq->direction = direction ^ 1;
		wheelLeftRunReq->rotations = 0.02f;

		if(!msgSend(g_robotQueue, getQueueIdFromTaskId(Msg_WheelsTaskID), (void**) &wheelLeftRunReq, portMAX_DELAY))
		{
			while(1) {}
		}

		WheelRunMsgReq* wheelRightRunReq = (WheelRunMsgReq*) pvPortMalloc(sizeof(WheelRunMsgReq));
		*wheelRightRunReq = INIT_WHEEL_RUN_MSG_REQ;
		wheelRightRunReq->wheelId = 1;
		wheelRightRunReq->direction = direction;
		wheelRightRunReq->rotations = 0.02f;

		if(!msgSend(g_robotQueue, getQueueIdFromTaskId(Msg_WheelsTaskID), (void**) &wheelRightRunReq, portMAX_DELAY))
		{
			while(1) {}
		}
//		for(int i = 0; i != motorsNum; ++i)
//		{
//		MotorSetDirectionMsgReq* directionReq = (MotorSetDirectionMsgReq*) pvPortMalloc(sizeof(MotorSetDirectionMsgReq));
//
//		*directionReq = INIT_MOTOR_SET_DIRECTION_MSG_REQ;
//		directionReq->motorId = i;
//		directionReq->direction = direction;
//
//		if(!msgSend(g_robotQueue, getQueueIdFromTaskId(Msg_MotorsTaskID), (void**) &directionReq, portMAX_DELAY))
//		{
//			while(1) {}
//		}
//		}
//
//		for(int8_t dutyCycle = 2; dutyCycle <= 100; dutyCycle++)
//		{
//			for(int i = 0; i != motorsNum; ++i)
//			{
//			MotorSetDutyCycleMsgReq* request = (MotorSetDutyCycleMsgReq*) pvPortMalloc(sizeof(MotorSetDutyCycleMsgReq));
//
//			*request = INIT_MOTOR_SET_DUTY_CYCLE_MSG_REQ;
//			request->motorId = i;
//			request->dutyCycle = dutyCycle;
//
//			if(!msgSend(g_robotQueue, getQueueIdFromTaskId(Msg_MotorsTaskID), (void**) &request, portMAX_DELAY))
//			{
//				while(1) {}
//			}
//			}
//
//			vTaskDelayUntil(&ui32WakeTime, 100 / portTICK_RATE_MS);
//		}
//
//		for(int8_t dutyCycle = 100; dutyCycle > 1; dutyCycle--)
//		{
//			for(int i = 0; i != motorsNum; ++i)
//			{
//			MotorSetDutyCycleMsgReq* request = (MotorSetDutyCycleMsgReq*) pvPortMalloc(sizeof(MotorSetDutyCycleMsgReq));
//
//			*request = INIT_MOTOR_SET_DUTY_CYCLE_MSG_REQ;
//			request->motorId = i;
//			request->dutyCycle = dutyCycle;
//
//			if(!msgSend(g_robotQueue, getQueueIdFromTaskId(Msg_MotorsTaskID), (void**) &request, portMAX_DELAY))
//			{
//				while(1) {}
//			}
//			}
//
//			vTaskDelayUntil(&ui32WakeTime, 100 / portTICK_RATE_MS);
//		}
//		GpioExpSetPin(gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN6);
//		GpioExpClearPin(gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN7);
//		vTaskDelayUntil(&ui32WakeTime, 50 / portTICK_RATE_MS);
//		GpioExpClearPin(gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN6);
//		GpioExpSetPin(gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN7);
//		vTaskDelayUntil(&ui32WakeTime, 50 / portTICK_RATE_MS);
//
		vTaskDelayUntil(&ui32WakeTime, pdMS_TO_TICKS(2000));
		direction ^= 1;

	}
*/
#endif

}

bool initRobotTask()
{
	g_robotQueue = registerMainMsgQueue(Msg_RobotTaskID, ROBOT_TASK_QUEUE_SIZE);

    if(xTaskCreate(robotTask, (signed portCHAR *)"ROBOT", ROBOT_TASK_STACK_SIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_ROBOT_TASK, NULL) != pdTRUE)
    {
        return false;
    }

    return true;
}

void initializeFPU()
{
    FPUEnable();
    FPULazyStackingEnable();
}

void initializeSysClock()
{
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       SYSCTL_OSC_MAIN);
}

void enableInterrupts()
{
	// Enable processor interrupts.
	IntMasterEnable();
}

bool initializeEEPROM()
{
	if(!SysCtlPeripheralReady(SYSCTL_PERIPH_EEPROM0))
		SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);

	return EEPROMInit() == EEPROM_INIT_OK;
}


//void initializeGpioExpanders()
//{
//	ZeroBuffer(&g_gpioExpander, sizeof(GpioExpander));
//
//	// setup gpio expander
//	g_gpioExpander.hwAddress		= 0x20;
//	g_gpioExpander.i2cManager 		= &g_i2cManager;
//	GpioExpInit(&g_gpioExpander);
//}

void initilizeFreeRTOS()
{
	vPortDefineHeapRegions( xHeapRegions );
}


bool startSpiServerCom()
{
	StartTaskMsgReq* request = (StartTaskMsgReq*) pvPortMalloc(sizeof(StartTaskMsgReq));
	*request = INIT_START_TASK_MSG_REQ;
	msgSend(g_robotQueue, getAddressFromTaskId(Msg_ServerSpiComTaskID), &request, MSG_WAIT_LONG_TIME);

	StartTaskMsgRsp* response;
	if(!msgReceive(g_robotQueue, &response, MSG_WAIT_LONG_TIME))
		return false;

	bool result = response->status;
	vPortFree(response);
	return result;
}

bool startUpdater()
{
	StartTaskMsgReq* request = (StartTaskMsgReq*) pvPortMalloc(sizeof(StartTaskMsgReq));
	*request = INIT_START_TASK_MSG_REQ;
	msgSend(g_robotQueue, getAddressFromTaskId(Msg_UpdaterTaskID), &request, MSG_WAIT_LONG_TIME);

	StartTaskMsgRsp* response;
	if(!msgReceive(g_robotQueue, &response, MSG_WAIT_LONG_TIME))
		return false;

	bool result = response->status;
	vPortFree(response);
	return result;
}


#ifdef _ROBOT_MASTER_BOARD
bool startWheels()
{
	StartTaskMsgReq* request = (StartTaskMsgReq*) pvPortMalloc(sizeof(StartTaskMsgReq));
	*request = INIT_START_TASK_MSG_REQ;
	msgSend(g_robotQueue, getAddressFromTaskId(Msg_WheelsTaskID), &request, MSG_WAIT_LONG_TIME);

	StartTaskMsgRsp* response;
	if(!msgReceive(g_robotQueue, &response, MSG_WAIT_LONG_TIME))
		return false;

	bool result = response->status;
	vPortFree(response);
	return result;
}

bool startMpu()
{
	StartTaskMsgReq* request = (StartTaskMsgReq*) pvPortMalloc(sizeof(StartTaskMsgReq));
	*request = INIT_START_TASK_MSG_REQ;
	msgSend(g_robotQueue, getAddressFromTaskId(Msg_MpuTaskID), &request, MSG_WAIT_LONG_TIME);

	StartTaskMsgRsp* response;
	if(!msgReceive(g_robotQueue, &response, MSG_WAIT_LONG_TIME))
		return false;

	bool result = response->status;
	vPortFree(response);
	return result;
}

bool startMotionControl()
{
	StartTaskMsgReq* request = (StartTaskMsgReq*) pvPortMalloc(sizeof(StartTaskMsgReq));
	*request = INIT_START_TASK_MSG_REQ;
	msgSend(g_robotQueue, getAddressFromTaskId(Msg_MotionControlTaskID), &request, MSG_WAIT_LONG_TIME);

	StartTaskMsgRsp* response;
	if(!msgReceive(g_robotQueue, &response, MSG_WAIT_LONG_TIME))
		return false;

	bool result = response->status;
	vPortFree(response);
	return result;
}

bool startMotors()
{
	StartTaskMsgReq* request = (StartTaskMsgReq*) pvPortMalloc(sizeof(StartTaskMsgReq));
	*request = INIT_START_TASK_MSG_REQ;
	msgSend(g_robotQueue, getAddressFromTaskId(Msg_MotorsTaskID), &request, MSG_WAIT_LONG_TIME);

	StartTaskMsgRsp* response;
	if(!msgReceive(g_robotQueue, &response, MSG_WAIT_LONG_TIME))
		return false;

	bool result = response->status;
	vPortFree(response);
	return result;
}

#else
bool startTcpServer()
{
	StartTaskMsgReq* request = (StartTaskMsgReq*) pvPortMalloc(sizeof(StartTaskMsgReq));
	*request = INIT_START_TASK_MSG_REQ;
	msgSend(g_robotQueue, getAddressFromTaskId(Msg_TcpServerTaskID), &request, MSG_WAIT_LONG_TIME);

	StartTaskMsgRsp* response;
	if(!msgReceive(g_robotQueue, &response, MSG_WAIT_LONG_TIME))
		return false;

	bool result = response->status;
	vPortFree(response);
	return result;
}
#endif



void jumpToAddress( uint32_t address )
{
	__asm("    ldr     sp, [r0]\n"
		  "    ldr     pc, [r0, #4]");
}

void onReset()
{
	uint32_t resetCause = SysCtlResetCauseGet();
	SysCtlResetCauseClear(resetCause);

	if(resetCause & SYSCTL_CAUSE_SW)
	{
		HWREG(NVIC_VTABLE) = 0;
		jumpToAddress(0);

	}
	else if(resetCause & SYSCTL_CAUSE_WDOG0)
	{
		logger(Error, Log_Robot, "[onReset] Reset cause: watchdog");
		g_isWdgReset = true;

		if(!isConfigRead())
			readConfig();

		bool isConfChanged = false;

		if(config.binary1.version > config.binary2.version)
		{
			if(HWREG(NVIC_VTABLE) == config.binary1.address)
			{
				if(!config.binary1.isChecked)
				{
					config.binary1.isChecked = true;
					config.binary1.isGood = false;
					isConfChanged = true;
				}
			}
		}
		else
		{
			if(HWREG(NVIC_VTABLE) == config.binary2.address)
			{
				if(!config.binary2.isChecked)
				{
					config.binary2.isChecked = true;
					config.binary2.isGood = false;
					isConfChanged = true;
				}
			}
		}

		if(isConfChanged)
		{
			writeConfig();

			HWREG(NVIC_VTABLE) = 0;
			jumpToAddress(0);
		}
	}
	else if(resetCause & SYSCTL_CAUSE_BOR)
	{
		logger(Error, Log_Robot, "[onReset] Reset cause: brown-out");
	}
	else
	{
		logger(Error, Log_Robot, "[onReset] Reset cause: %d", resetCause);
	}
}

void afterWdgReset()
{
#ifdef _ROBOT_MASTER_BOARD
	LedInit(LED6);
	LedTurnOff(LED6);
#endif

	if(g_isWdgReset)
	{
		setIsDumpPM(false);

#ifdef _ROBOT_MASTER_BOARD

		LedBlinkStart(LED6, 2);
#endif
	}

}


// workaround for linking errors (relates to getTaskList FreeRTOS)
char * _sbrk( size_t x )
{
    /* Just to remove compiler warning. */
    ( void ) x;
    return NULL;
}

