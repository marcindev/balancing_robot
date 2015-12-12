#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	uint8_t* bufferStart;
	uint8_t* bufferEnd;
	uint8_t* head;
	uint8_t* tail;
	uint16_t size;
}CircularBuffer;

void CB_setBuffer(CircularBuffer* circularBuffer, uint8_t* buffer, uint32_t size);
bool CB_isEmpty(CircularBuffer* circularBuffer);
bool CB_isFull(CircularBuffer* circularBuffer);
bool CB_pushData(CircularBuffer* circularBuffer, const void* data, uint32_t size);
uint32_t CB_popData(CircularBuffer* circularBuffer, void* data, uint32_t size);
uint32_t CB_getAvailableSpace(CircularBuffer* circularBuffer);

#endif // CIRCULAR_BUFFER_H
