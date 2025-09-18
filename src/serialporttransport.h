#ifndef SERIALPORTTRANSPORT_H
#define SERIALPORTTRANSPORT_H

#include "transport.h"
#include <QSerialPort>
#include <QTimer>

class SerialPortTransport : public Transport
{
    Q_OBJECT
public:
    explicit SerialPortTransport(QObject *parent = nullptr);
    ~SerialPortTransport();

    bool open(const SerialSettings &settings) override;
    void close() override;
    bool isOpen() const override;
    void send(const QByteArray &data) override;

private slots:
    void onReadyRead();
    void onFrameTimeout();
    void onError(QSerialPort::SerialPortError error);

private:
    QSerialPort *port;
    QTimer frameTimer;
    QByteArray buffer;
};

#endif // SERIALPORTTRANSPORT_H
