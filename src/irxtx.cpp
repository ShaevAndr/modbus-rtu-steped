#include "irxtx.h"
#include <QTimer>
#include <QDebug>

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
        qDebug()<<"we couldnt open the port";
        emit errorOccurred(port->errorString());
        return false;
    }

    connect(port, &QSerialPort::readyRead, this, &IRxTx::onReadyRead);
    connect(&frameTimer, &QTimer::timeout, this, &IRxTx::onFrameTimeout);
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
    buffer.append(port->readAll());
    // каждый раз сбрасываем таймер при новых данных
    frameTimer.start(5); // 5 ms > 3.5 символа при 9600 бод
}

void IRxTx::onFrameTimeout() {
    qDebug() << "Кадр принят:" << buffer.toHex(' ');

    // TODO: здесь должна быть проверка CRC и разбор ответа
    if (buffer.size() >= 5) {
        qDebug() << "Адрес:" << static_cast<unsigned char>(buffer[0])
                 << "Функция:" << static_cast<unsigned char>(buffer[1]);
    }

    buffer.clear();
}

void IRxTx::onError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError)
        emit errorOccurred(port->errorString());
}
