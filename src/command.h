#ifndef COMMAND_H
#define COMMAND_H

#include <QByteArray>

struct Command
{
    enum CommandType {
        Prepare, // Подготовительная команда (например, для чтения длины)
        Main     // Основная команда для чтения/записи данных
    };

    CommandType type = Main;
    quint8 deviceAddress;
    quint8 functionCode;
    QByteArray data;

    // Можно добавить конструкторы для удобства
    Command(quint8 addr, quint8 func, const QByteArray &d = QByteArray(), CommandType t = Main)
        : type(t), deviceAddress(addr), functionCode(func), data(d) {}
};

#endif // COMMAND_H