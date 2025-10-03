#ifndef IPROTOCOL_H
#define IPROTOCOL_H

#include <QByteArray>
#include <QString>
#include <QVector>
#include <QDebug>
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

    virtual QVector<Command> setParameterI(quint8 deviceAddr, quint16 index, quint16 value) = 0;
    virtual QVector<Command> setParameterF(quint8 deviceAddr, quint16 index, float value) = 0;
    virtual QVector<Command> setParametersI(quint8 deviceAddr, quint16 index, QVector<quint16> values) = 0;
    virtual QVector<Command> setParametersF(quint8 deviceAddr, quint16 index, QVector<float> values) = 0;
    virtual QVector<Command> getParametersI(quint8 deviceAddr, quint16 index, quint16 count) = 0;
    virtual QVector<Command> getParametersF(quint8 deviceAddr, quint16 index, quint16 count) = 0;
    virtual QVector<Command> getDeviceInfo(quint8 deviceAddr) = 0;
};

#endif // IPROTOCOL_H
