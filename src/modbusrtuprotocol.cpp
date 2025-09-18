#include "modbusrtuprotocol.h"
#include "modbuscrc.h"

ModbusRtuProtocol::ModbusRtuProtocol()
{

}

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
