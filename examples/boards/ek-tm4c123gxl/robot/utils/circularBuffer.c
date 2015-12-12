#include "circularBuffer.h"


void CB_setBuffer(CircularBuffer* circularBuffer, uint8_t* buffer, uint32_t size)
{
	circularBuffer->size = size;
	circularBuffer->bufferStart = buffer;
	circularBuffer->bufferEnd = circularBuffer->bufferStart + size;
	circularBuffer->head = circularBuffer->tail = circularBuffer->bufferStart;

}

bool CB_isEmpty(CircularBuffer* circularBuffer)
{
	return circularBuffer->head == circularBuffer->tail;
}

bool CB_isFull(CircularBuffer* circularBuffer)
{
	return (circularBuffer->tail - circularBuffer->head) == 1
			|| (circularBuffer->head - circularBuffer->tail) == (circularBuffer->size - 1);
}

bool CB_pushData(CircularBuffer* circularBuffer, const void* data, uint32_t size)
{
	if(CB_getAvailableSpace(circularBuffer) < size)
		return false;

	uint8_t* u8Data = (uint8_t*) data;

	uint8_t* prevHead = circularBuffer->head;

	for(uint32_t i = 0; i < size; ++i)
	{
		*circularBuffer->head = *u8Data++;

		if((circularBuffer->head + 1) == circularBuffer->bufferEnd)
			circularBuffer->head = circularBuffer->bufferStart;
		else
			circularBuffer->head++;

		if(circularBuffer->head == circularBuffer->tail)
		{
			circularBuffer->head = prevHead;
			return false;
		}
	}

	return true;
}

uint32_t CB_popData(CircularBuffer* circularBuffer, void* data, uint32_t size)
{
	uint32_t popCnt = 0;
	uint8_t* u8Data = (uint8_t*) data;

	for(uint32_t i = 0; i < size; ++i)
	{
		if(CB_isEmpty(circularBuffer))
			return popCnt;

		*u8Data++ = *circularBuffer->tail;
		circularBuffer->tail++;

		if(circularBuffer->tail == circularBuffer->bufferEnd)
			circularBuffer->tail = circularBuffer->bufferStart;

		++popCnt;

	}

	return popCnt;
}

uint32_t CB_getAvailableSpace(CircularBuffer* circularBuffer)
{
	if(circularBuffer->head < circularBuffer->tail)
		return circularBuffer->tail - 1 - circularBuffer->head;

	return ((circularBuffer->bufferEnd - 1) - circularBuffer->head)
			+ (circularBuffer->tail - circularBuffer->bufferStart);
}
