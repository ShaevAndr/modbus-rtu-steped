#ifndef REQUEST_H
#define REQUEST_H

#include <functional>
#include <QVector>
#include "command.h"
#include "IProtocol.h" // For Response struct

struct Request
{
    QVector<Command> commands;
    std::function<void(const QVector<Response>&)> onSuccess;
    std::function<void(const QString&)> onError;
};

#endif // REQUEST_H
