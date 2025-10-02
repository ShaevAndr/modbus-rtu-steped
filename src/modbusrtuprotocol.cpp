#include "modbusrtuprotocol.h"
#include "modbuscrc.h"
#include <QDataStream>

ModbusRtuProtocol::ModbusRtuProtocol()
{

}

static constexpr int MAX_REGS_PER_FRAME = 120;

QByteArray ModbusRtuProtocol::encode(const Command &cmd)
{
    QByteArray frame;
    frame.append(cmd.deviceAddress);
    frame.append(cmd.functionCode);
    frame.append(cmd.data);

    quint16 crc = ModbusCRC::calculate(reinterpret_cast<const unsigned char*>(frame.constData()), frame.size());
    frame.append(static_cast<char>(crc & 0xFF));
    frame.append(static_cast<char>((crc >> 8) & 0xFF));

    return frame;
}

#include "errorcodes.h"
#include <QDebug>

Response ModbusRtuProtocol::decode(const QByteArray &frame)
{
    Response response;
    response.errorCode = ecNoError; // По умолчанию нет ошибки

    // Минимальный размер кадра: Адрес(1) + Код функции(1) + CRC(2)
    if (frame.size() < 4) {
        response.errorCode = ecBadLength;
        response.errorMessage = "Incomplete frame received";
        return response;
    }

    // Проверка CRC
    quint16 receivedCrc = (static_cast<quint8>(frame[frame.size() - 1]) << 8) |
                           static_cast<quint8>(frame[frame.size() - 2]);
    QByteArray dataToCheck = frame.left(frame.size() - 2);
    quint16 calculatedCrc = ModbusCRC::calculate(
        reinterpret_cast<const unsigned char*>(dataToCheck.constData()), dataToCheck.size());

    if (receivedCrc != calculatedCrc) {
        response.errorCode = ecBadCS;
        response.errorMessage = "CRC mismatch";
        return response;
    }

    quint8 address = static_cast<quint8>(frame[0]);
    quint8 functionCode = static_cast<quint8>(frame[1]);

    // Проверка на Modbus исключение (старший бит кода функции установлен)
    if (functionCode & 0x80) {
        response.errorCode = ecBadAnswer;
        quint8 exceptionCode = static_cast<quint8>(frame[2]);
        response.errorMessage = "Modbus exception code: " + QString::number(exceptionCode);
        response.data = frame.mid(2, 1); // код ошибки
    } else {
        // Обработка стандартного ответа чтения данных (например, функция 0x03)
        if (frame.size() >= 5) {
            quint8 byteCount = static_cast<quint8>(frame[2]);
            if (frame.size() >= static_cast<int>(3 + byteCount + 2)) { // +2 для CRC
                response.data = frame.mid(3, byteCount);
            } else {
                response.errorCode = ecBadLength;
                response.errorMessage = "Byte count mismatch";
            }
        } else {
            response.errorCode = ecBadLength;
            response.errorMessage = "Frame too short for data";
        }
    }

    qDebug() << "[ModbusRtuProtocol] Decoded data:" << response.data.toHex(' ')
             << "Error code:" << response.errorCode
             << response.errorMessage;

    return response;
}


QVector<Command> ModbusRtuProtocol::setParameterI(quint8 deviceAddr, quint16 index, quint16 value) {
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::BigEndian);

    s << index;
    s << quint16(1);
    s << value;

    return { Command(deviceAddr, WRITE_MULTIPLE_REGS, Command::ReasponseDataType::Int, data) };
}

// Один float (2 регистра)
QVector<Command> ModbusRtuProtocol::setParameterF(quint8 deviceAddr, quint16 index, float value) {
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::BigEndian);

    quint32 raw;
    memcpy(&raw, &value, sizeof(float));

    quint16 high = static_cast<quint16>((raw >> 16) & 0xFFFF);
    quint16 low  = static_cast<quint16>(raw & 0xFFFF);

    s << index;           // адрес
    s << quint16(2);      // количество регистров
    s << quint8(4);       // количество байт
    s << high;
    s << low;

    return { Command(deviceAddr, WRITE_MULTIPLE_REGS, Command::ReasponseDataType::Int, data) };
}

// Несколько int
QVector<Command> ModbusRtuProtocol::setParametersI(quint8 deviceAddr, quint16 index, QVector<quint16> values) {
    QVector<Command> commands;
    int offset = 0;

    while (offset < values.size()) {
        int chunkSize = qMin<int>(WRITE_MULTIPLE_REGS, values.size() - offset);

        QByteArray data;
        QDataStream s(&data, QIODevice::WriteOnly);
        s.setByteOrder(QDataStream::BigEndian);

        s << quint16(index + offset);
        s << quint16(chunkSize);
        s << quint8(chunkSize * 2);

        for (int i = 0; i < chunkSize; i++) {
            s << values[offset + i];
        }

        commands.append(Command(deviceAddr, WRITE_MULTIPLE_REGS, Command::ReasponseDataType::Int, data));
        offset += chunkSize;
    }

    return commands;
}

// Несколько float
QVector<Command> ModbusRtuProtocol::setParametersF(quint8 deviceAddr, quint16 index, QVector<float> values) {
    QVector<Command> commands;
    int offset = 0;

    while (offset < values.size()) {
        int chunkSize = qMin<int>(MAX_REGS_PER_FRAME / 2, values.size() - offset);
        // /2 потому что 1 float = 2 регистра

        QByteArray data;
        QDataStream s(&data, QIODevice::WriteOnly);
        s.setByteOrder(QDataStream::BigEndian);

        s << quint16(index + offset * 2);   // сдвиг по регистрам
        s << quint16(chunkSize * 2);        // кол-во регистров
        s << quint8(chunkSize * 4);         // кол-во байт данных

        for (int i = 0; i < chunkSize; i++) {
            quint32 raw;
            memcpy(&raw, &values[offset + i], sizeof(float));

            quint16 high = static_cast<quint16>((raw >> 16) & 0xFFFF);
            quint16 low  = static_cast<quint16>(raw & 0xFFFF);

            s << high;
            s << low;
        }

        commands.append(Command(deviceAddr, WRITE_MULTIPLE_REGS, Command::ReasponseDataType::Int, data));
        offset += chunkSize;
    }

    return commands;
}

// ------------------ GET ------------------


// Несколько int
QVector<Command> ModbusRtuProtocol::getParametersI(quint8 deviceAddr, quint16 index, quint16 count) {
    QVector<Command> commands;
    quint16 offset = 0;

    while (offset < count) {
        quint16 chunkSize = qMin<quint16>(MAX_REGS_PER_FRAME, count - offset);

        QByteArray data;
        QDataStream s(&data, QIODevice::WriteOnly);
        s.setByteOrder(QDataStream::BigEndian);

        s << quint16(index + offset);
        s << chunkSize;

        commands.append(Command(deviceAddr, READ_HOLDING_REGS, Command::ReasponseDataType::Int, data));
        offset += chunkSize;
    }

    return commands;
}

// Несколько float
QVector<Command> ModbusRtuProtocol::getParametersF(quint8 deviceAddr, quint16 index, quint16 count) {
    QVector<Command> commands;
    quint16 totalRegs = count * 2;
    quint16 offset = 0;

    while (offset < totalRegs) {
        quint16 chunkSize = qMin<quint16>(MAX_REGS_PER_FRAME, totalRegs - offset);

        QByteArray data;
        QDataStream s(&data, QIODevice::WriteOnly);
        s.setByteOrder(QDataStream::BigEndian);

        s << quint16(index + offset);
        s << chunkSize;

        commands.append(Command(deviceAddr, READ_HOLDING_REGS, Command::ReasponseDataType::Float, data));
        offset += chunkSize;
    }

    return commands;
}

QVector<Command> ModbusRtuProtocol::getDeviceInfo(quint8 deviceAddr) {
    QVector<Command> commands;
    QByteArray data;
    data.append("\x0E\x00", 2);
    commands.append(Command(deviceAddr, ENCAPSULATED_INTERFACE, Command::ReasponseDataType::Char, data));
    return commands;
}
