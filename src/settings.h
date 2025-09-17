#pragma once
#include <QString>
#include <QSerialPort>

struct SerialSettings {
    QString portName;
    int baudRate = 9600;
    QSerialPort::Parity parity = QSerialPort::NoParity;
    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    QSerialPort::StopBits stopBits = QSerialPort::OneStop;
    int timeoutMs = 500;
};
