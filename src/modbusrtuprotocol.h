#ifndef MODBUSRTUPROTOCOL_H
#define MODBUSRTUPROTOCOL_H

#include "IProtocol.h"


class ModbusRtuProtocol : public IProtocol
{
public:
    ModbusRtuProtocol();

    enum ModbusFunction {
        READ_COILS               = 0x01, // Чтение дискретных выходов (Coils)
        READ_DISCRETE_INPUTS     = 0x02, // Чтение дискретных входов
        READ_HOLDING_REGS        = 0x03, // Чтение holding-регистров
        READ_INPUT_REGS          = 0x04, // Чтение input-регистров
        WRITE_SINGLE_COIL        = 0x05, // Запись одного дискретного выхода
        WRITE_SINGLE_REG         = 0x06, // Запись одного holding-регистра
        READ_EXCEPTION_STATUS    = 0x07, // Чтение статуса исключений (только RTU)
        DIAGNOSTICS              = 0x08, // Диагностика

        WRITE_MULTIPLE_COILS     = 0x0F, // Запись нескольких дискретных выходов
        WRITE_MULTIPLE_REGS      = 0x10, // Запись нескольких holding-регистров

        REPORT_SERVER_ID         = 0x11, // Возврат идентификатора устройства
        READ_FILE_RECORD         = 0x14, // Чтение файла
        WRITE_FILE_RECORD        = 0x15, // Запись файла
        MASK_WRITE_REG           = 0x16, // Маскирование бита в регистре
        READ_WRITE_MULTIPLE_REGS = 0x17, // Чтение/запись нескольких регистров
        READ_FIFO_QUEUE          = 0x18, // Чтение FIFO очереди

        ENCAPSULATED_INTERFACE   = 0x2B  // Encapsulated Interface Transport
    };

    QByteArray encode(const Command &cmd) override;
    Response decode(const QByteArray &frame) override;

    Command parameterI(int ecode, int index, int value) override;
    Command parameterF(int ecode, int index, float value) override;
    Command parametersI(int ecode, QVector<int> values) override;
    Command parametersF(int ecode, QVector<float> values) override;
};

#endif // MODBUSRTUPROTOCOL_H
