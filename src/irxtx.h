#pragma once
#include <QObject>
#include <QSerialPort>
#include "settings.h"

class IRxTx : public QObject
{
    Q_OBJECT
public:
    explicit IRxTx(QObject *parent = nullptr);
    ~IRxTx();

    bool open(const SerialSettings &settings);
    void close();
    bool isOpen() const;
    void send(const QByteArray &data);

signals:
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &err);

private slots:
    void onReadyRead();
    void onError(QSerialPort::SerialPortError error);

private:
    QSerialPort *port;
};
