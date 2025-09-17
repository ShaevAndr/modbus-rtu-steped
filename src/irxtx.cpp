#include "irxtx.h"

IRxTx::IRxTx(QObject *parent) : QObject(parent), port(new QSerialPort(this))
{
    connect(port, &QSerialPort::readyRead, this, &IRxTx::onReadyRead);
    connect(port, &QSerialPort::errorOccurred, this, &IRxTx::onError);
}

IRxTx::~IRxTx()
{
    close();
}

bool IRxTx::open(const SerialSettings &settings)
{
    if (port->isOpen()) port->close();
    port->setPortName(settings.portName);
    port->setBaudRate(settings.baudRate);
    port->setParity(settings.parity);
    port->setDataBits(settings.dataBits);
    port->setStopBits(settings.stopBits);

    if (!port->open(QIODevice::ReadWrite)) {
        emit errorOccurred(port->errorString());
        return false;
    }
    return true;
}

void IRxTx::close()
{
    if (port->isOpen()) port->close();
}

bool IRxTx::isOpen() const
{
    return port->isOpen();
}

void IRxTx::send(const QByteArray &data)
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

void IRxTx::onReadyRead()
{
    QByteArray d = port->readAll();
    emit dataReceived(d);
}

void IRxTx::onError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError)
        emit errorOccurred(port->errorString());
}
