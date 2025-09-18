#ifndef MODBUSRTUPROTOCOL_H
#define MODBUSRTUPROTOCOL_H

#include "IProtocol.h"

class ModbusRtuProtocol : public IProtocol
{
public:
    ModbusRtuProtocol();

    QByteArray encode(const Command &cmd) override;
    Response decode(const QByteArray &frame) override;
};

#endif // MODBUSRTUPROTOCOL_H
