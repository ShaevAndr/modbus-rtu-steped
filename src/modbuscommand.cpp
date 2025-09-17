#include "modbuscommand.h"

class ModbusCommand {
public:
    QByteArray frame;   // Готовый кадр Modbus
    quint8 slaveAddr;
    quint8 functionCode;

    std::function<void(const QByteArray&)> onSuccess;
    std::function<void(const QString&)> onError;
};
