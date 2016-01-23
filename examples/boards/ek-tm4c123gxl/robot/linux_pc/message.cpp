#include "message.h"

#include "../messages/messages.h"

Message::Message()
{

}

Message::Message(std::shared_ptr<void> _payload) : payload(_payload)
{

}

size_t Message::getSize()
{
	return getMsgSize(payload.get());
}


std::shared_ptr<void> Message::getPayload()
{
	return payload;
}

void* Message::getRawPayload()
{
	return payload.get();
}
