#ifndef MESSAGE_H
#define MESSAGE_H

#include <memory>

class Message
{
public:
	Message();
	Message(std::shared_ptr<void> _payload);

	size_t getSize();
	std::shared_ptr<void> getPayload();
	void* getRawPayload();
private:
	std::shared_ptr<void> payload;
};



#endif // MESSAGE_H
