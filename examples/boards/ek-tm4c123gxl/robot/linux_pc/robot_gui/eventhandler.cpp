#include "eventhandler.h"

EventHandler::EventHandler(QObject *parent) : QObject(parent)
{

}


void EventHandler::connectedCallback()
{
    emit connected();
}

void EventHandler::disconnectedCallback()
{
    emit disconnected();
}

void EventHandler::getLogsFinishedCallback()
{
    emit getLogsFinished();
}

void EventHandler::getLogsLineReceivedCallback()
{
    emit getLogsLineReceived();
}

void EventHandler::setPidParamFinishedCallback()
{
    emit setPidParamFinished();
}

void EventHandler::setPidParamTimeoutCallback()
{
    emit setPidParamTimeout();
}
