#include "led.h"
#include "MCP23017.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "global_defs.h"
#include "logger.h"

#define LEDS_NUM            10
#define LEDS_TIMER_PERIOD   500

typedef struct
{
    GpioExpander* gpioExpander;
    uint8_t gpioPort;
    uint8_t gpioPin;
    uint32_t period;
    bool isOn;
    bool initialized;
} LedData;


static LedData g_leds[LEDS_NUM];
static  TimerHandle_t g_ledsTimer;

static void initializeLedTimer();
static void ledsTimerCallback(TimerHandle_t pxTimer);


void LedInit(Led led)
{
    if(g_leds[led].initialized)
        return;

    I2cManager* i2cManager = (I2cManager*) pvPortMalloc(sizeof(I2cManager));
    g_leds[led].gpioExpander = (GpioExpander*) pvPortMalloc(sizeof(GpioExpander));

    ZeroBuffer(g_leds[led].gpioExpander, sizeof(GpioExpander));
    g_leds[led].gpioExpander->i2cManager = i2cManager;
    g_leds[led].gpioExpander->hwAddress     = GPIO_EXPANDER2_ADDRESS;


    switch(led)
    {
    case LED1:
    case LED2:
    case LED3:
    case LED4:
    case LED5:
    case LED6:
    case LED7:
        g_leds[led].gpioPort = GPIOEXP_PORTB;
        break;
    case LED8:
    case LED9:
    case LED10:
    case LED11:
        g_leds[led].gpioPort = GPIOEXP_PORTA;
        break;
    }

    switch(led)
    {
    case LED1:
        g_leds[led].gpioPin = GPIOEXP_PIN7;
        break;
    case LED2:
        g_leds[led].gpioPin = GPIOEXP_PIN6;
        break;
    case LED3:
        g_leds[led].gpioPin = GPIOEXP_PIN5;
        break;
    case LED4:
        g_leds[led].gpioPin = GPIOEXP_PIN4;
        break;
    case LED5:
        g_leds[led].gpioPin = GPIOEXP_PIN3;
        break;
    case LED6:
        g_leds[led].gpioPin = GPIOEXP_PIN2;
        break;
    case LED7:
        g_leds[led].gpioPin = GPIOEXP_PIN1;
        break;
    case LED8:
        g_leds[led].gpioPin = GPIOEXP_PIN0;
        break;
    case LED9:
        g_leds[led].gpioPin = GPIOEXP_PIN1;
        break;
    case LED10:
        g_leds[led].gpioPin = GPIOEXP_PIN2;
        break;
    case LED11:
        g_leds[led].gpioPin = GPIOEXP_PIN3;
        break;
    }

    GpioExpInit(g_leds[led].gpioExpander);
    GpioExpSetPinDirOut(g_leds[led].gpioExpander, g_leds[led].gpioPort, g_leds[led].gpioPin);

    g_leds[led].isOn = false;
    g_leds[led].initialized = true;

    logger(Info, Log_Leds, "[LedInit] LED%d initialized", led + 1);
}


void LedTurnOn(Led led)
{
    if(!g_leds[led].initialized)
        LedInit(led);

    GpioExpSetPin(g_leds[led].gpioExpander, g_leds[led].gpioPort, g_leds[led].gpioPin);
}

void LedTurnOff(Led led)
{
    if(!g_leds[led].initialized)
        LedInit(led);

    GpioExpClearPin(g_leds[led].gpioExpander, g_leds[led].gpioPort, g_leds[led].gpioPin);
}

void LedTurnOnOff(Led led, bool isOn)
{
    isOn ? LedTurnOn(led) : LedTurnOff(led);
}

void LedBlinkStart(Led led, uint32_t period)
{
    if(!g_ledsTimer)
        initializeLedTimer();

    LedTurnOff(led);
    g_leds[led].period = period;
}

void LedBlinkStop(Led led)
{
    LedBlinkStart(led, 0);
}


void initializeLedTimer()
{

    g_ledsTimer = xTimerCreate(
            "LedTimer",
            pdMS_TO_TICKS(LEDS_TIMER_PERIOD),
            pdTRUE,
            ( void * ) 0,
            ledsTimerCallback
    );

    if(g_ledsTimer == NULL)
    {
        logger(Error, Log_Leds, "[initializeLedTimer] Couldn't create timer");
        return;
    }

    logger(Debug, Log_Leds, "[initializeLedTimer] timer created");

    if( xTimerStart( g_ledsTimer, 0 ) != pdPASS )
    {
        logger(Error, Log_Leds, "[initializeLedTimer] Couldn't start timer");
    }

    logger(Debug, Log_Leds, "[initializeLedTimer] timer started");
}

void ledsTimerCallback(TimerHandle_t pxTimer)
{
    static uint32_t counter;
    counter++;

    for(int led = 0; led != LEDS_NUM; led++)
    {
        if(g_leds[led].period && !(counter % g_leds[led].period))
        {
            g_leds[led].isOn = !g_leds[led].isOn;
            LedTurnOnOff(led, g_leds[led].isOn);
        }
    }

}

