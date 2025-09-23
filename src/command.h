#ifndef COMMAND_H
#define COMMAND_H

#include <QByteArray>

struct Command
{
    enum class CommandType {
        Prepare, // Подготовительная команда (например, для чтения длины)
        Main     // Основная команда для чтения/записи данных
    };

    enum class ReasponseDataType {
        Float,
        Int,
        Double
    };

    CommandType type = CommandType::Main;
    QByteArray frame;
    quint8 deviceAddress;
    quint8 functionCode;
    QByteArray data;
    ReasponseDataType dataType = ReasponseDataType::Int;

    Command() = default;

    Command(quint8 addr,
            quint8 func,
            ReasponseDataType dt,
            const QByteArray &d = QByteArray(),
            CommandType t = CommandType::Main)
        : type(t),
          deviceAddress(addr),
          functionCode(func),
          data(d),
          dataType(dt) {}
};

#endif // COMMAND_H
