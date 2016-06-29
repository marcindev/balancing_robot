#ifndef EVANTHANDLER_H
#define EVANTHANDLER_H

#include <QObject>

class EventHandler : public QObject
{
    Q_OBJECT
public:
    explicit EventHandler(QObject *parent = 0);

    void connectedCallback();
    void disconnectedCallback();
    void getLogsFinishedCallback();
    void getLogsLineReceivedCallback();
    void setPidParamFinishedCallback();
    void setPidParamTimeoutCallback();

signals:
    void connected();
    void disconnected();
    void getLogsFinished();
    void getLogsLineReceived();
    void setPidParamFinished();
    void setPidParamTimeout();

public slots:
};

#endif // EVANTHANDLER_H
