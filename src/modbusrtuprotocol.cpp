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
    qDebug() << "address" << frame.toHex();
    frame.append(cmd.functionCode);
    qDebug() << frame.toHex();
    frame.append(cmd.data);

    quint16 crc = ModbusCRC::calculate(reinterpret_cast<const unsigned char*>(frame.constData()), frame.size());
    frame.append(static_cast<char>(crc & 0xFF));
    frame.append(static_cast<char>((crc >> 8) & 0xFF));

    return frame;
}

Response ModbusRtuProtocol::decode(const QByteArray &frame)
{
    Response response;

    // Минимальный размер кадра Modbus: Адрес(1) + Код(1) + CRC(2) = 4
    if (frame.size() < 4) {
        response.errorCode = ecBadLength;
        response.errorMessage = "Incomplete frame received";
        return response;
    }

    // Проверка CRC
    quint16 receivedCrc = (static_cast<quint8>(frame[frame.size() - 1]) << 8) | static_cast<quint8>(frame[frame.size() - 2]);
    QByteArray dataToCheck = frame.left(frame.size() - 2);
    quint16 calculatedCrc = ModbusCRC::calculate(reinterpret_cast<const unsigned char*>(dataToCheck.constData()), dataToCheck.size());

    if (receivedCrc != calculatedCrc) {
        response.errorCode = ecBadCS;
        response.errorMessage = "CRC mismatch";
        return response;
    }

    // Проверка на ошибку Modbus (старший бит кода функции установлен)
    if (frame[1] & 0x80) {
        response.errorCode = ecBadAnswer;
        response.errorMessage = "Modbus exception code: " + QString::number(frame[2]);
        response.data = frame.mid(2, 1); // Код ошибки
    } else {
        // Данные находятся между кодом функции и CRC
        response.data = frame.mid(2, frame.size() - 4);
    }

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
