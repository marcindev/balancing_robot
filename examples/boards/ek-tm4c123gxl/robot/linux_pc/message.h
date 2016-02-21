#ifndef MESSAGE_H
#define MESSAGE_H

#include <memory>

class BaseMessage
{
public:
	static std::shared_ptr<BaseMessage> getMessageFromPayload(unsigned char* data);
	BaseMessage() { }
	virtual ~BaseMessage() { }
	virtual size_t getSize() = 0;
	virtual void* getRawPayload() = 0;
	virtual unsigned char getMsgId() = 0;
};

template <typename T>
class Message : public BaseMessage
{
public:
	Message() { }
	Message(std::shared_ptr<T> _payload) : payload(_payload) { }
	Message(BaseMessage& baseMsg)
	{
		payload = dynamic_cast<Message<T>&>(baseMsg).payload;
	}

	&operator T() { return *payload; }

	size_t getSize() { return getMsgSize(payload.get()); }
	std::shared_ptr<T> getPayload() { return payload; }
	void* getRawPayload() { return reinterpret_cast<void*>(payload.get()); }
	unsigned char getMsgId() { return *(reinterpret_cast<unsigned char*>(payload.get())); }
private:
	std::shared_ptr<T> payload;
};



#endif // MESSAGE_H
