#include "udmaWrapper.h"
#include "driverlib/sysctl.h"
#include "memory.h"

typedef struct
{
    bool isInitialized;
} UdmaCtrl;

struct _UdmaChannel
{
    uint32_t chNumber;   
    void* src;
    void* dest;
    uint32_t size;
    bool isSrcIncr;
    bool isDestIncr;
};

// The control table used by the uDMA controller.  This table must be aligned
// to a 1024 byte boundary.
#ifdef _ROBOT_MASTER_BOARD
uint8_t ui8ControlTable[1024] __attribute__ ((aligned(1024)));
#else
uint8_t ui8ControlTable[512] __attribute__ ((aligned(512)));
#endif


static UdmaCtrl udmaCtrl;


void UdmaInit()
{
    if(udmaCtrl.isInitialized)
	return;

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
//  IntEnable(INT_UDMAERR);
    uDMAEnable();
    uDMAControlBaseSet(ui8ControlTable);

    udmaCtrl.isInitialized = true;
}

void UdmaDeinit()
{
    uDMADisable();

    udmaCtrl.isInitialized = false;
}

uint32_t UdmaGetErrorStatus()
{
    return uDMAErrorStatusGet();
}

void UdmaClearErrorStatus()
{
    uDMAErrorStatusClear();
}

UdmaChannel UdmaCreateChannel(uint32_t channelNumber, bool isSrcIncr, bool isDestIncr)
{
    UdmaChannel channel = (UdmaChannel) malloc(sizeof(struct _UdmaChannel));
    channel->chNumber = channelNumber;
    channel->isSrcIncr = isSrcIncr;
    channel->isDestIncr = isDestIncr;

    return channel;
}

void UdmaReleaseChannel(UdmaChannel channel)
{
    if(UdmaIsTransferOngoing(channel))
	UdmaStopTransfer(channel);

    free(channel);
}

void UdmaChannelInit(UdmaChannel channel)
{

    uDMAChannelAttributeDisable(channel->chNumber,
                                UDMA_ATTR_USEBURST |
                                UDMA_ATTR_HIGH_PRIORITY |
                                UDMA_ATTR_ALTSELECT |
                                UDMA_ATTR_REQMASK);



    uDMAChannelAttributeEnable(channel->chNumber, UDMA_ATTR_USEBURST);

    uint32_t srcIncr = channel->isSrcIncr ? UDMA_SRC_INC_8 : UDMA_SRC_INC_NONE;
    uint32_t destIncr = channel->isDestIncr ? UDMA_DST_INC_8 : UDMA_DST_INC_NONE;

    uDMAChannelControlSet(channel->chNumber | UDMA_PRI_SELECT,
                          UDMA_SIZE_8 | 
			  srcIncr |
                          destIncr | UDMA_ARB_4);
}

bool UdmaIsTransferOngoing(UdmaChannel channel)
{
    return uDMAChannelIsEnabled(channel->chNumber);
}

// Buffers cannot be read/written to/from flash
void UdmaSetBuffers(UdmaChannel channel, void* src, void* dest, uint32_t size)
{
    channel->src = src;
    channel->dest = dest;
    channel->size = size;

}

void UdmaStartTransfer(UdmaChannel channel)
{
    uDMAChannelTransferSet(channel->chNumber | UDMA_PRI_SELECT,
                            UDMA_MODE_BASIC,
                            channel->src,
                            channel->dest,
                            channel->size);

    uDMAChannelEnable(channel->chNumber);
}

void UdmaStopTransfer(UdmaChannel channel)
{
    uDMAChannelDisable(channel->chNumber);

    return true;
}




