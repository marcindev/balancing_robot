#ifndef MEMORY_DEVICE_H
#define MEMORY_DEVICE_H 
#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    MEM_DEV_READY,
    MEM_DEV_BUSY,
    MEM_DEV_ERROR,
    MEM_DEV_INVALID_PARAM,
    MEM_INIT_PARAMS_NOT_SET
} MemDevStatus;

typedef struct
{
    uint32_t begAddr;
    uint32_t endAddr;
    uint32_t smallBlockSize;
    uint32_t largeBlockSize;
    uint32_t eraseBlockSize;
} MemDevParams;

typedef struct _MemoryDevice* MemoryDevice;
typedef uint32_t MemAddress;
typedef void MemData;
typedef uint32_t MemSize;
typedef int32_t(*MemRead)(MemAddress, MemData*, MemSize);
typedef int32_t(*MemWrite)(MemAddress, MemData*, MemSize);
typedef int32_t(*MemErase)(MemAddress, MemSize);
typedef int32_t(*MemInit)();
typedef int32_t(*MemGetParams)(MemDevParams*);

void MemDevRegisterInitFunc(MemoryDevice memoryDevice, MemInit initFunc);
void MemDevRegisterGetParamsFunc(MemoryDevice memoryDevice, MemGetParams getParamsFunc);
void MemDevRegisterReadFunc(MemoryDevice memoryDevice, MemRead readFunc);
void MemDevRegisterWriteFunc(MemoryDevice memoryDevice, MemWrite writeFunc);
void MemDevRegisterEraseFunc(MemoryDevice memoryDevice, MemErase eraseFunc);
MemoryDevice MemDevCreate();
bool MemDevOpen(MemoryDevice memoryDevice);
bool MemDevClose(MemoryDevice memoryDevice);
bool MemDevRead(MemoryDevice memoryDevice, uint32_t address, void* data, uint32_t size);
bool MemDevWrite(MemoryDevice memoryDevice, uint32_t address, void* data, uint32_t size);
MemDevStatus  MemDevGetStatus(MemoryDevice memoryDevice);

#endif // MEMORY_DEVICE_H
