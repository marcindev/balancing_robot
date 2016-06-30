
#include "FreeRTOS.h"
#include "task.h"
#include "portable.h"
#include "logger.h"
#include <stdarg.h>
#include "driverlib/eeprom.h"
#include "inc/hw_types.h"
#include <unwind.h>

#define BUFFER_SIZE         30

#define INT_ARG             1
#define DOUBLE_ARG          2

#define MAX_ARGS_NUM        10

#define PM_ELEM_BEG         0xF1
#define PM_STOP_MARK        0xBF
#define PM_LEN_OFFSET       1
#define PM_BEG_BLOCK        2

#define ISR_BUFFERS_SIZE    4

typedef struct
{
    uint32_t timestampMilis;
    LogLevel logLevel;
    LogComponent component;
    void* stringPtr;
    uint8_t argsNum;
    uint8_t* argTypes;
    uint8_t* argsBuffer;
    uint8_t argsBuffSize;
}LogElem;

typedef struct
{
    uint8_t isrArgTypesBuff[10];
    uint8_t isrArgsBuff[80];
}IsrBuffers;

static LogElem g_buffer[BUFFER_SIZE];
static uint16_t index = 0;
static bool isRollOver = false;

static IsrBuffers isrBuffers[ISR_BUFFERS_SIZE];
static uint8_t isrBuffIndex = 0;
static uint32_t g_framePointer;
static uint8_t g_stackDepth;
static char g_stacktraceStr[120];


static void _logStackTrace(uint32_t* addresses, uint8_t num);
static void getFramePointer() __attribute__( ( naked ) );
static void _getFramePointer(uint32_t fp);

void logger(LogLevel level, LogComponent component, const char* string, ...)
{
    va_list arguments;

    if(index == BUFFER_SIZE)
    {
        index = 0;
        isRollOver = true;
    }
    g_buffer[index].timestampMilis = xTaskGetTickCount() / portTICK_RATE_MS;
    g_buffer[index].logLevel = level;
    g_buffer[index].component = component;
    g_buffer[index].stringPtr = string;

    uint32_t i = 0;
    uint8_t argsNum = 0;
    uint8_t argTypes[MAX_ARGS_NUM] = {0};
    bool isInArg = false;

    while(string[i] != 0)
    {
        if((i > 0) && !isInArg && (string[i] == '%') && (string[i - 1] != '%'))
        {
            argsNum++;
            isInArg = true;
        }
        else if(isInArg)
        {
            switch(string[i])
            {
            case 'd':
            case 'i':
            case 'u':
            case 'o':
            case 'x':
            case 'X':
            case 'c':
            case 'p':
            case 'n':
                isInArg = false;
                argTypes[argsNum - 1] = INT_ARG;
                break;

            case 'f':
            case 'F':
            case 'e':
            case 'E':
            case 'g':
            case 'G':
            case 'a':
            case 'A':
                isInArg = false;
                argTypes[argsNum - 1] = DOUBLE_ARG;
                break;

            default:
                break;
            }
        }

        ++i;
    }

    g_buffer[index].argsNum = argsNum;


    bool isInIsr = (portNVIC_INT_CTRL_REG & 0xFFUL) != 0;


    if(argsNum > 0)
    {
        uint32_t argsBuffSize = 0;
        uint8_t* argTypesPtr = 0;

        if(isInIsr)
            argTypesPtr = isrBuffers[isrBuffIndex].isrArgTypesBuff;
        else
            argTypesPtr = (uint8_t*) pvPortMalloc(argsNum);

        for(int j = 0; j != argsNum; ++j)
        {
            argTypesPtr[j] = argTypes[j];

            if(argTypes[j] == DOUBLE_ARG)
                argsBuffSize += sizeof(double);
            else
                argsBuffSize += sizeof(uint32_t);

        }
        g_buffer[index].argsBuffSize = argsBuffSize;

        uint8_t* argsBuff = 0;

        if(isInIsr)
            argsBuff = isrBuffers[isrBuffIndex].isrArgsBuff;
        else
            argsBuff = (uint8_t*) pvPortMalloc(argsBuffSize);

        va_start(arguments, argsNum);
        uint8_t* argsBuffPtr = argsBuff;

        for(int j = 0; j != argsNum; ++j)
        {
            uint8_t* bytePtr = 0;
            uint8_t argSize = 0;

            if(argTypes[j] == DOUBLE_ARG)
            {
                double arg = (double) va_arg(arguments, double);
                bytePtr = (uint8_t*) &arg;
                argSize = sizeof(double);

                for(int k = 0; k != argSize; ++k)
                {
                    *argsBuffPtr = *bytePtr;
                    ++argsBuffPtr;
                    ++bytePtr;
                }
            }
            else
            {
                uint32_t arg = (uint32_t) va_arg(arguments, uint32_t);
                bytePtr = (uint8_t*) &arg;
                argSize = sizeof(uint32_t);

                for(int k = 0; k != argSize; ++k)
                {
                    *argsBuffPtr = *bytePtr;
                    ++argsBuffPtr;
                    ++bytePtr;
                }
            }

            if(isInIsr)
                isrBuffIndex++;

        }

        va_end ( arguments );

        if(isRollOver)
        {
            if(!isInIsr && g_buffer[index].argsBuffer)
                vPortFree(g_buffer[index].argsBuffer);

            if(!isInIsr && g_buffer[index].argTypes)
                vPortFree(g_buffer[index].argTypes);
        }

        g_buffer[index].argsBuffer = argsBuff;
        g_buffer[index].argTypes = argTypesPtr;
    }
    else
    {
        if(!isInIsr && g_buffer[index].argsBuffer)
            vPortFree(g_buffer[index].argsBuffer);

        if(!isInIsr && g_buffer[index].argTypes)
            vPortFree(g_buffer[index].argTypes);

        g_buffer[index].argsBuffer = 0;
        g_buffer[index].argTypes = 0;
    }



    index++;
}

bool getNextLogLine(uint32_t* timestamp, LogLevel* logLevel, LogComponent* component,
                    void** strPtr, uint8_t* argsNum, void** argTypes, void** argsBufferPtr,
                    uint8_t* argsBuffSize)
{
    if((index == 0) && (!isRollOver))
        return false;

    static isStart = true;
    static uint16_t nextInd = 0;
    static totalLinesNum = 0;
    static lineNum = 0;

    if(isStart)
    {
        nextInd = (isRollOver ? index : 0);
        totalLinesNum = getLinesNumber();
        lineNum = 1;
    }

    if(nextInd == BUFFER_SIZE)
        nextInd = 0;

    *timestamp = g_buffer[nextInd].timestampMilis;
    *logLevel = g_buffer[nextInd].logLevel;
    *component = g_buffer[nextInd].component;
    *strPtr = g_buffer[nextInd].stringPtr;
    *argsNum = g_buffer[nextInd].argsNum;
    *argTypes = g_buffer[nextInd].argTypes;
    *argsBufferPtr = g_buffer[nextInd].argsBuffer;
    *argsBuffSize = g_buffer[nextInd].argsBuffSize;

    if(!isStart && ((nextInd == index) || (lineNum > totalLinesNum)))
    {
        isStart = true;
        return false;
    }
    else
    {
        isStart = false;
    }

    nextInd++;
    lineNum++;

    return true;
}

uint16_t getLinesNumber()
{
    return (isRollOver ?  BUFFER_SIZE : index);
}

bool dumpPostMortem()
{
    const uint16_t MAX_ELEM_BUFF_SIZE = 30;
    const uint16_t WORD_SIZE = sizeof(uint32_t);
    uint32_t pmBuffer[MAX_ELEM_BUFF_SIZE];

    memset(pmBuffer, 0, MAX_ELEM_BUFF_SIZE * WORD_SIZE);

    int nextInd = index - 1;
    uint32_t stopWord = 0;
    *((uint8_t*)&stopWord) = PM_STOP_MARK;

    uint32_t blockSize = EEPROMSizeGet() / EEPROMBlockCountGet();
    int32_t leftFlashSpace = EEPROMSizeGet() - blockSize * PM_BEG_BLOCK;
    uint32_t currFlashAddr = EEPROMAddrFromBlock(PM_BEG_BLOCK);

    while(true)
    {
        uint8_t *pmBufferBeg = (uint8_t*)pmBuffer,
                *pmBufferPtr = (uint8_t*)pmBuffer;

        if(nextInd < 0)
        {
            if(isRollOver)
            {
                nextInd = BUFFER_SIZE - 1;
            }
            else
            {
                EEPROMProgram(&stopWord, currFlashAddr, WORD_SIZE);
                break;
            }
        }

        if(nextInd == index)
        {
            EEPROMProgram(&stopWord, currFlashAddr, WORD_SIZE);
            break;
        }

        *pmBufferPtr++ = PM_ELEM_BEG;
        pmBufferPtr++; // leave off for further length value
        *pmBufferPtr++ = g_buffer[nextInd].logLevel;
        *pmBufferPtr++ = g_buffer[nextInd].component;
        *((uint32_t*)pmBufferPtr)= g_buffer[nextInd].timestampMilis;
        pmBufferPtr += WORD_SIZE;
        *((uint32_t*)pmBufferPtr) = g_buffer[nextInd].stringPtr;
        pmBufferPtr += WORD_SIZE;
        uint8_t argsNum = g_buffer[nextInd].argsNum;
        *pmBufferPtr++ =  argsNum;
        memcpy(pmBufferPtr, g_buffer[nextInd].argTypes, argsNum);
        pmBufferPtr += argsNum;
        uint8_t argsBuffSize = g_buffer[nextInd].argsBuffSize;
        *pmBufferPtr++ = argsBuffSize;
        memcpy(pmBufferPtr, g_buffer[nextInd].argsBuffer, argsBuffSize);
        pmBufferPtr += argsBuffSize;

        uint16_t u8WriteSize = pmBufferPtr - pmBufferBeg;
        uint16_t u32WriteSize = u8WriteSize / WORD_SIZE;

        if(u8WriteSize % WORD_SIZE)
            u32WriteSize++;

        u8WriteSize = u32WriteSize * WORD_SIZE;

        *(pmBufferBeg + PM_LEN_OFFSET) = u8WriteSize;

        leftFlashSpace -= u8WriteSize;

        if(leftFlashSpace <= WORD_SIZE)
        {
            EEPROMProgram(&stopWord, currFlashAddr, WORD_SIZE);
            break;
        }

        if(EEPROMProgram(pmBuffer, currFlashAddr, u8WriteSize))
            return false;

        currFlashAddr += u8WriteSize;
        nextInd--;
    }

    return true;
}

bool getNextPMLine(uint32_t* timestamp, LogLevel* logLevel, LogComponent* component,
                    void** strPtr, uint8_t* argsNum, void* argTypes, void* argsBufferPtr,
                    uint8_t* argsBuffSize)
{
    const uint16_t MAX_ELEM_BUFF_SIZE = 30;
    uint16_t WORD_SIZE = sizeof(uint32_t);
    const uint8_t MAX_ARGS_BUFFER_SIZE = 64 * 10;
    uint32_t pmBuffer[MAX_ELEM_BUFF_SIZE];

    memset(pmBuffer, 0, MAX_ELEM_BUFF_SIZE * WORD_SIZE);

    static uint32_t blockSize = 0;
    static int32_t leftFlashSpace = 0;
    static uint32_t currFlashAddr = 0;
    static isStart = true;

    blockSize = EEPROMSizeGet() / EEPROMBlockCountGet();

    uint8_t* pmBufferPtr = (uint8_t*)pmBuffer;

    if(isStart)
    {
        currFlashAddr = EEPROMAddrFromBlock(PM_BEG_BLOCK);
        leftFlashSpace = EEPROMSizeGet() - blockSize * PM_BEG_BLOCK;
        isStart = false;
    }

    while(true)
    {
        EEPROMRead(pmBufferPtr, currFlashAddr, WORD_SIZE);
        leftFlashSpace -= WORD_SIZE;
        currFlashAddr += WORD_SIZE;

        if(*pmBufferPtr == PM_ELEM_BEG)
            break;

        if(*pmBufferPtr == PM_STOP_MARK || leftFlashSpace <= WORD_SIZE)
        {
            isStart = true;
            return false;
        }
    }

    pmBufferPtr++;

    uint8_t elemLen = *pmBufferPtr++;

    if(elemLen - WORD_SIZE > leftFlashSpace)
    {
        isStart = true;
        return false;
    }

    *logLevel = *pmBufferPtr++;
    *component = *pmBufferPtr++;

    EEPROMRead(pmBufferPtr, currFlashAddr, elemLen - WORD_SIZE);
    leftFlashSpace -= elemLen - WORD_SIZE;
    currFlashAddr += elemLen - WORD_SIZE;

    *timestamp = *((uint32_t*)pmBufferPtr);
    pmBufferPtr += WORD_SIZE;
    *strPtr = *((uint32_t*)pmBufferPtr);
    pmBufferPtr += WORD_SIZE;
    *argsNum = *pmBufferPtr++;

    if(*argsNum > MAX_ARGS_NUM)
        *argsNum = MAX_ARGS_NUM;

    memcpy(argTypes, pmBufferPtr, *argsNum);
    pmBufferPtr += *argsNum;

    *argsBuffSize = *pmBufferPtr++;

    if(*argsBuffSize > MAX_ARGS_BUFFER_SIZE)
        *argsBuffSize = MAX_ARGS_BUFFER_SIZE;

    memcpy(argsBufferPtr, pmBufferPtr, *argsBuffSize);
    pmBufferPtr += *argsBuffSize;


    return true;
}

_Unwind_Reason_Code backtraceCallback(_Unwind_Context *ctx, void* addresses)
{

    *((uint32_t*) addresses + g_stackDepth) = _Unwind_GetIP(ctx);
    g_stackDepth++;
    return _URC_NO_REASON;
}

void logStackTrace()
{
    const uint32_t RAM_START = 0x20000000;
    const uint32_t RAM_END = 0x20008000;
    const uint8_t DEPTH = 30;

//  getFramePointer();
//
//  uint32_t fp = g_framePointer;
//
//  if(!fp)
//      return;
//
//  uint32_t pc = 0, lr = 0;
//  uint8_t depth = DEPTH;
//
//  if(fp < RAM_START || fp > RAM_END)
//  {
//      logger(Error, Log_Robot, "[logStackTrace]: fp out of range");
//      return;
//  }

    uint32_t funcAddresses[DEPTH];
    memset(funcAddresses, 0, DEPTH);

    g_stackDepth = 0;

    _Unwind_Backtrace(&backtraceCallback, funcAddresses);
//  do
//  {
//      pc = HWREG(fp - 16);
//      fp = HWREG(fp - 12);
//
//      funcAddresses[i++] = pc;
//
//      if(--depth == 0)
//          break;
//
//      if(fp < RAM_START || fp > RAM_END)
//          break;
//
//  }while(pc != (uint32_t) main || pc != (uint32_t) ResetISR);
//

    _logStackTrace(funcAddresses, g_stackDepth);
}

void getFramePointer()
{
    __asm volatile
    (
        " mov r0, r7                                               \n"
        " ldr r1, handler                                           \n"
        " bx r1                                                     \n"
        " handler: .word _getFramePointer                           \n"
    );
}

void _getFramePointer(uint32_t fp)
{
    g_framePointer = fp;
}

void _logStackTrace(uint32_t* addresses, uint8_t num)
{
    if(!num)
        return;

    if(index == BUFFER_SIZE)
    {
        index = 0;
        isRollOver = true;
    }

    g_buffer[index].timestampMilis = xTaskGetTickCount() / portTICK_RATE_MS;
    g_buffer[index].logLevel = Info;
    g_buffer[index].component = Log_Robot;
    g_buffer[index].argsNum = num;

    bool isInIsr = (portNVIC_INT_CTRL_REG & 0xFFUL) != 0;

    const char* strPrefix = "[_logStackTrace] Stacktrace: ";
    const char* strElement = "[%#.8x]->";

    size_t strPrefixLen = strlen(strPrefix);
    size_t strElementLen = strlen(strElement);

    size_t logStrLen = strPrefixLen + num * strElementLen + 1;

    char* logStr = NULL;

    if(isInIsr)
    {
        logStr = &g_stacktraceStr;
    }
    else
    {
        logStr = (char*) pvPortMalloc(logStrLen);

        if(!logStr)
        {
            logStr = &g_stacktraceStr;
            isInIsr = true;
        }
    }

    char* strPtr = logStr;

    memcpy(strPtr, strPrefix, strPrefixLen);
    strPtr += strPrefixLen;

    for(uint8_t i = 0; i != num; ++i)
    {
        memcpy(strPtr, strElement, strElementLen);
        strPtr += strElementLen;
    }

    *strPtr = '\0';
    g_buffer[index].stringPtr = logStr;

    uint8_t* argTypesPtr = 0;

    if(isInIsr)
        argTypesPtr = isrBuffers[isrBuffIndex].isrArgTypesBuff;
    else
        argTypesPtr = (uint8_t*) pvPortMalloc(num);

    if(isRollOver)
    {
        if(!isInIsr && g_buffer[index].argsBuffer)
            vPortFree(g_buffer[index].argsBuffer);

        if(!isInIsr && g_buffer[index].argTypes)
            vPortFree(g_buffer[index].argTypes);
    }

    memset(argTypesPtr, INT_ARG, num);
    g_buffer[index].argTypes = argTypesPtr;
    uint8_t argsBuffSize = num * sizeof(uint32_t);
    g_buffer[index].argsBuffSize = argsBuffSize;

    uint8_t* argsBufferPtr = 0;

    if(isInIsr)
        argsBufferPtr = isrBuffers[isrBuffIndex].isrArgsBuff;
    else
        argsBufferPtr = (uint8_t*) pvPortMalloc(argsBuffSize);

    memcpy(argsBufferPtr, addresses, argsBuffSize);

    g_buffer[index].argsBuffer = argsBufferPtr;

    if(isInIsr)
        isrBuffIndex++;

    index++;

}

void abort(void)
{
  for (;;) { }
}

