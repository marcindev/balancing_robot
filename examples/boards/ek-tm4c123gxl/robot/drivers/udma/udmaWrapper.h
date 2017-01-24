#ifndef UDMA_WRAPPER_H
#define UDMA_WRAPPER_H
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/udma.h"

typedef struct _UdmaChannel* UdmaChannel;

void UdmaInit();
void UdmaDeinit();
uint32_t UdmaGetErrorStatus();
void UdmaClearErrorStatus();
UdmaChannel UdmaCreateChannel(uint32_t channelNumer, bool isSrcIncr, bool isDestIncr);
void UdmaReleaseChannel(UdmaChannel channel);
void UdmaChannelInit(UdmaChannel channel);
bool UdmaIsTransferOngoing(UdmaChannel channel);
void UdmaSetBuffers(UdmaChannel channel, void* src, void* dest, uint32_t size);
void UdmaStartTransfer(UdmaChannel channel);
void UdmaStopTransfer(UdmaChannel channel);


#endif // UDMA_WRAPPER_H
