#include "memory.h"
#include "memoryDevice.h"
#include "FreeRTOS.h"
#include "portable.h"
#include "utils/utils.h"
#include <string.h>

#define READ_FUNC_REGISTERED		    0x01
#define WRITE_FUNC_REGISTERED		    0x02
#define PAGE_SIZE_SET			    0x04
#define RANGE_SET			    0x08
#define MEMORY_ZEROED			    0x10
#define DEVICE_INITIALIZED  READ_FUNC_REGISTERED|WRITE_FUNC_REGISTERED|PAGE_SIZE_SET \
			    |RANGE_SET|MEMORY_ZEROED

struct _MemoryDevice
{
    bool isOpen;
    bool isBusy;
    uint32_t smallBlockSize;
    uint32_t largeBlockSize;
    uint32_t beg;
    uint32_t end;
    uint32_t eraseBlockSize;
    MemInit init;
    MemGetParams getParams;
    MemRead read;
    MemWrite write;
    MemErase erase;
    bool isInit;
};


static int32_t defaultInit()
{
    return -1;
}

static int32_t defaultGetMemDevParams(MemDevParams* params)
{
    params->begAddr = 0;
    params->endAddr = 0;
    params->smallBlockSize = 0;
    params->largeBlockSize = 0;
    params->eraseBlockSize = 0;

    return -1;
}

static int32_t defaultRead(MemAddress addr, MemData* data, MemSize size)
{
    return -1;
}

static int32_t defaultWrite(MemAddress addr, MemData* data, MemSize size)
{
    return -1;
}

static int32_t defaultErase(MemAddress addr, MemSize size)
{
    return -1;
}



static void initialize(MemoryDevice memoryDevice)
{
    MemDevParams params;
    memoryDevice->getParams(&params);
    memoryDevice->beg = params.begAddr;
    memoryDevice->end = params.endAddr;
    memoryDevice->smallBlockSize = params.smallBlockSize;
    memoryDevice->largeBlockSize = params.largeBlockSize;
    memoryDevice->eraseBlockSize = params.eraseBlockSize;

    memoryDevice->init();

    memoryDevice->isInit = true;
}

void MemDevRegisterInitFunc(MemoryDevice memoryDevice, MemInit initFunc)
{
    memoryDevice->init = initFunc; 
}

void MemDevRegisterGetParamsFunc(MemoryDevice memoryDevice, MemGetParams getParamsFunc)
{
    memoryDevice->getParams = getParamsFunc; 
}

void MemDevRegisterReadFunc(MemoryDevice memoryDevice, MemRead readFunc)
{
    memoryDevice->read = readFunc;
}

void MemDevRegisterWriteFunc(MemoryDevice memoryDevice, MemWrite writeFunc)
{
    memoryDevice->write = writeFunc;
}

void MemDevRegisterEraseFunc(MemoryDevice memoryDevice, MemErase eraseFunc)
{
    memoryDevice->erase = eraseFunc;
}

MemoryDevice MemDevCreate()
{
    MemoryDevice device = (MemoryDevice) malloc(sizeof(_MemoryDevice));
    ZeroBuffer(device, sizeof(_MemoryDevice));

    device->init = defaultInit;
    device->getParams = defaultGetMemDevParams;
    device->read = defaultRead;
    device->write = defaultWrite;
    device->erase = defaultErase;

    return device;
}

bool MemDevOpen(MemoryDevice memoryDevice)
{
    // TODO return file descriptor

    if(memoryDevice->isOpen)
	return false;

    if(!memoryDevice->isInit)
	initialize(memoryDevice);

    memoryDevice->isOpen = true;

    return true;
}

bool MemDevClose(MemoryDevice memoryDevice)
{
    if(!memoryDevice->isOpen)
	return false;

    memoryDevice->isOpen = false;

    return true;
}

bool MemDevRead(MemoryDevice memoryDevice, uint32_t address, void* data, uint32_t size)
{
    if(!memoryDevice->isOpen)
	return false;

    if(address < memoryDevice->beg || address > memoryDevice->end)
	return false;

    uint32_t blockSize = (size < memoryDevice->largeBlockSize / 2) 
		       ? memoryDevice->smallBlockSize : memoryDevice->largeBlockSize;

    uint32_t begBlockIndex  = (address - memoryDevice->beg) / blockSize;
    uint32_t begBlockOffset = (address - memoryDevice->beg) % blockSize;
    uint32_t endBlockIndex  = (address - memoryDevice->beg + size) / blockSize;
    uint32_t endBlockOffset = (address - memoryDevice->beg + size) % blockSize;
    uint8_t tempData[blockSize];

    uint8_t* destPtr = NULL, *srcPtr = NULL;
    uint32_t len = 0;

    MemAddress srcAddr = memoryDevice->beg + begBlockIndex * blockSize;
    len = blockSize;
   
    if(memoryDevice->read(srcAddr, tempData, len) < 0)
	return false;

    srcPtr = tempData + begBlockOffset;	
    destPtr = (uint8_t*) data;

    if(endBlockIndex == begBlockIndex)
    {
	len = endBlockOffset - begBlockOffset;
	memcpy(data, srcPtr, len);
	return true;
    }
    else
    {
	len = blockSize - begBlockOffset;	 
	memcpy(data, srcPtr, len);
	destPtr += len;
    }

    srcAddr = memoryDevice->beg + endBlockIndex * blockSize;
    len = blockSize;

    if(memoryDevice->read(srcAddr, tempData, len) < 0)
	return false;

    srcPtr = tempData;
    uint8_t* prevDestPtr = destPtr;
    destPtr += (endBlockIndex - begBlockIndex - 1) * blockSize;
    memcpy(destPtr, srcPtr, endBlockOffset);

    if((endBlockIndex - begBlockIndex) == 1)
	return true;

    uint32_t fullBlocksNum = endBlockIndex - begBlockIndex - 1;

    destPtr = prevDestPtr;
    srcAddr = memoryDevice->beg + (begBlockIndex + 1) * blockSize;
    len = fullBlocksNum * blockSize;

    if(memoryDevice->read(srcAddr, destPtr, len) < 0)
	return false;

    return true;
}

bool MemDevWrite(MemoryDevice memoryDevice, uint32_t address, void* data, uint32_t size)
{
    if(!memoryDevice->isOpen)
	return false;

    if(address < memoryDevice->beg || address > memoryDevice->end)
	return false;

    uint32_t blockSize = (size < memoryDevice->largeBlockSize / 2) 
		       ? memoryDevice->smallBlockSize : memoryDevice->largeBlockSize;

    uint32_t begBlockIndex  = (address - memoryDevice->beg) / blockSize;
    uint32_t begBlockOffset = (address - memoryDevice->beg) % blockSize;
    uint32_t endBlockIndex  = (address - memoryDevice->beg + size) / blockSize;
    uint32_t endBlockOffset = (address - memoryDevice->beg + size) % blockSize;
    uint8_t tempData[blockSize];

    uint8_t* destPtr = NULL, *srcPtr = NULL;
    uint32_t len = 0;

    MemAddress srcAddr = memoryDevice->beg + begBlockIndex * blockSize;
    len = blockSize;

    if(memoryDevice->read(srcAddr, tempData, len) < 0)
	return false;

    destPtr = tempData + begBlockOffset;	
    MemAddress destAddr = 0;

    if(endBlockIndex == begBlockIndex)
    {
	len = endBlockOffset - begBlockOffset;
	memcpy(destPtr, data, len);

	destAddr = memoryDevice->beg + begBlockIndex * blockSize;
	len = blockSize;

	if(memoryDevice->write(destAddr, tempData, len) < 0)
	    return false;

	return true;
    }
    else
    {
	len = blockSize - begBlockOffset;	 
    	memcpy(destPtr, data, len);

	destAddr = memoryDevice->beg +  begBlockIndex * blockSize;
	len = blockSize;

	if(memoryDevice->write(destAddr, tempData, len) < 0)
	    return false;

    }

    srcAddr = memoryDevice->beg + endBlockIndex * blockSize;
    len = blockSize;

    if(memoryDevice->read(srcAddr, tempData, len) < 0)
	return false;

    srcPtr = (uint8_t*)data + blockSize - begBlockOffset + (endBlockIndex - begBlockIndex - 1) * blockSize;
    len = endBlockOffset;
    memcpy(tempData, srcPtr, len);

    destAddr = srcAddr;
    len = blockSize;

    if(memoryDevice->write(destAddr, tempData, len) < 0)
	return false;

    if((endBlockIndex - begBlockIndex) == 1)
	return true;

    uint32_t fullBlocksNum = endBlockIndex - begBlockIndex - 1;
    srcPtr = (uint8_t*) data + blockSize - begBlockOffset;
    destAddr = memoryDevice->beg + (begBlockIndex + 1) * blockSize;
    len = fullBlocksNum * blockSize;

    if(memoryDevice->write(destAddr, srcPtr, len) < 0)
	return false;

    return true;
}

bool MemDevErase(MemoryDevice memoryDevice, uint32_t address, uint32_t size)
{
    if(!memoryDevice->isOpen)
	return false;

    if(address < memoryDevice->beg || address > memoryDevice->end)
	return false;

    uint32_t blockSize = (size < memoryDevice->largeBlockSize / 2) 
		       ? memoryDevice->smallBlockSize : memoryDevice->largeBlockSize;

    uint32_t begBlockIndex  = (address - memoryDevice->beg) / blockSize;
    uint32_t begBlockOffset = (address - memoryDevice->beg) % blockSize;
    uint32_t endBlockIndex  = (address - memoryDevice->beg + size) / blockSize;
    uint32_t endBlockOffset = (address - memoryDevice->beg + size) % blockSize;
    uint8_t tempData[blockSize];

    uint8_t* destPtr = NULL, *srcPtr = NULL;
    uint32_t len = 0;

    MemAddress srcAddr = memoryDevice->beg + begBlockIndex * blockSize;
    len = blockSize;
   
    if(memoryDevice->read(srcAddr, tempData, len) < 0)
	return false;

    srcPtr = tempData + begBlockOffset;	
    destPtr = (uint8_t*) data;

    if(endBlockIndex == begBlockIndex)
    {
	len = endBlockOffset - begBlockOffset;
	memcpy(data, srcPtr, len);
	return true;
    }
    else
    {
	len = blockSize - begBlockOffset;	 
	memcpy(data, srcPtr, len);
	destPtr += len;
    }

    srcAddr = memoryDevice->beg + endBlockIndex * blockSize;
    len = blockSize;

    if(memoryDevice->read(srcAddr, tempData, len) < 0)
	return false;

    srcPtr = tempData;
    uint8_t* prevDestPtr = destPtr;
    destPtr += (endBlockIndex - begBlockIndex - 1) * blockSize;
    memcpy(destPtr, srcPtr, endBlockOffset);

    if((endBlockIndex - begBlockIndex) == 1)
	return true;

    uint32_t fullBlocksNum = endBlockIndex - begBlockIndex - 1;

    destPtr = prevDestPtr;
    srcAddr = memoryDevice->beg + (begBlockIndex + 1) * blockSize;
    len = fullBlocksNum * blockSize;

    if(memoryDevice->read(srcAddr, destPtr, len) < 0)
	return false;

    return true;

}

