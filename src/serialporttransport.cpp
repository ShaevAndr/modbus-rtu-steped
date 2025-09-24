#include "serialporttransport.h"
#include <QTimer>
#include <QDebug>

SerialPortTransport::SerialPortTransport(QObject *parent)
    : Transport(parent),
      port(new QSerialPort(this))
{
    // Подписка на сигналы один раз
    connect(port, &QSerialPort::readyRead,
            this, &SerialPortTransport::onReadyRead);
    connect(port, &QSerialPort::errorOccurred,
            this, &SerialPortTransport::onError);

    // Настраиваем таймер для разделения кадров
    frameTimer.setSingleShot(true);
    connect(&frameTimer, &QTimer::timeout,
            this, &SerialPortTransport::onFrameTimeout);
}

SerialPortTransport::~SerialPortTransport()
{
    close();
}

bool SerialPortTransport::open(const SerialSettings &settings)
{
    if (port->isOpen())
        port->close();

    port->setPortName(settings.portName);
    port->setBaudRate(settings.baudRate);
    port->setParity(settings.parity);
    port->setDataBits(settings.dataBits);
    port->setStopBits(settings.stopBits);

    if (!port->open(QIODevice::ReadWrite)) {
        qDebug() << "[SerialPortTransport] Не удалось открыть порт:"
                 << settings.portName << port->errorString();
        emit errorOccurred(port->errorString());
        return false;
    }

    qDebug() << "[SerialPortTransport] Порт открыт:" << settings.portName;
    return true;
}

void SerialPortTransport::close()
{
    frameTimer.stop();
    if (port->isOpen()) {
        qDebug() << "[SerialPortTransport] Закрытие порта";
        port->close();
    }
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

    qDebug() << "[SerialPortTransport] Отправка:" << data.toHex(' ');
    qint64 written = port->write(data);

    if (written == -1) {
        qDebug() << "[SerialPortTransport] Ошибка при отправке";
        emit errorOccurred(port->errorString());
    }

    port->flush();
}

void SerialPortTransport::onReadyRead()
{
    QByteArray data = port->readAll();
    if (!data.isEmpty()) {
        qDebug() << "[SerialPortTransport] Прочитано:" << data.toHex(' ');
        buffer.append(data);
    }

    // Сбрасываем одноразовый таймер
    frameTimer.start(5); // > 3.5 символа при 9600 бод
}

void SerialPortTransport::onFrameTimeout()
{
    if (!buffer.isEmpty()) {
        qDebug() << "[SerialPortTransport] Кадр готов:" << buffer.toHex(' ');
        emit dataReceived(buffer);
        buffer.clear();
    }
}

void SerialPortTransport::onError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError) {
        qDebug() << "[SerialPortTransport] Ошибка:" << error
                 << port->errorString();
        emit errorOccurred(port->errorString());
    }
}
