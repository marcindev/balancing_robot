#ifndef EVANTHANDLER_H
#define EVANTHANDLER_H

#include <QObject>

class EvantHandler : public QObject
{
    Q_OBJECT
public:
    explicit EvantHandler(QObject *parent = 0);

signals:

public slots:
};

#endif // EVANTHANDLER_H
