#ifndef IPROTOCOL_H
#define IPROTOCOL_H

#include <QByteArray>
#include <QString>
#include "command.h"
#include "errorcodes.h"

// Структура для унифицированного ответа
struct Response
{
    ECodes errorCode = ecNoError;
    QByteArray data;
    QString errorMessage;
};


class IProtocol
{
public:
    virtual ~IProtocol() = default;

    // Кодирует команду в байтовый массив для отправки
    virtual QByteArray encode(const Command &cmd) = 0;

    // Декодирует байтовый массив в структуру ответа
    virtual Response decode(const QByteArray &frame) = 0;

    virtual Command parameterI(int ecode, int index, int value) = 0;
    virtual Command parameterF(int ecode, int index, float value) = 0;
    virtual Command parametersI(int ecode, QVector<int> values) = 0;
    virtual Command parametersF(int ecode, QVector<float> values) = 0;
};

#endif // IPROTOCOL_H
