#include "serialporttransport.h"
#include <QTimer>
#include <QDebug>

SerialPortTransport::SerialPortTransport(QObject *parent) : Transport(parent), port(new QSerialPort(this))
{
    connect(port, &QSerialPort::readyRead, this, &SerialPortTransport::onReadyRead);
    connect(port, &QSerialPort::errorOccurred, this, &SerialPortTransport::onError);
}

SerialPortTransport::~SerialPortTransport()
{
    close();
}

bool SerialPortTransport::open(const SerialSettings &settings)
{
    if (port->isOpen()) port->close();
    port->setPortName(settings.portName);
    port->setBaudRate(settings.baudRate);
    port->setParity(settings.parity);
    port->setDataBits(settings.dataBits);
    port->setStopBits(settings.stopBits);

    if (!port->open(QIODevice::ReadWrite)) {
        qDebug()<<"we couldnt open the port";
        emit errorOccurred(port->errorString());
        return false;
    }

    connect(port, &QSerialPort::readyRead, this, &SerialPortTransport::onReadyRead);
    connect(&frameTimer, &QTimer::timeout, this, &SerialPortTransport::onFrameTimeout);
    return true;
}

void SerialPortTransport::close()
{
    if (port->isOpen()) port->close();
}

bool SerialPortTransport::isOpen() const
{
    return port->isOpen();
}

void SerialPortTransport::send(const QByteArray &data)
{
    if (!port->isOpen()) {
        emit errorOccurred("Port is not open");
        return;
    }
    qint64 written = port->write(data);
    if (written == -1) {
        emit errorOccurred(port->errorString());
    }
    port->flush();
}

void SerialPortTransport::onReadyRead()
{
    buffer.append(port->readAll());
    // каждый раз сбрасываем таймер при новых данных
    frameTimer.start(5); // 5 ms > 3.5 символа при 9600 бод
}

void SerialPortTransport::onFrameTimeout() {
    emit dataReceived(buffer);
    buffer.clear();
}

void SerialPortTransport::onError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError)
        emit errorOccurred(port->errorString());
}
