#include "callbacker.h"


unsigned Callbacker::Callback::nextId = 1;

Callbacker::Callback::Callback() : id(nextId++)
{

}

void Callbacker::execOnEvent(int event)
{
    auto it = eventCallbackMap[event].cbegin();

    for(; it != eventCallbackMap[event].cend(); ++it)
    {
        (*it)->execute();
    }
}

bool Callbacker::deregisterCallback(unsigned callbackId)
{
    for(auto& event : eventCallbackMap)
    {
        for(auto it = event.second.cbegin(); it != event.second.cend(); ++it)
        {
            if((*it)->getId() == callbackId)
            {
                event.second.erase(it);
                return true;
            }
        }
    }

    return false;
}

unsigned Callbacker::EventRegisterer::registerToCallbacker(Callbacker& callbacker) const
{
    return helper->registerWithParams(callbacker);
}
