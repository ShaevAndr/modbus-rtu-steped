#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <QObject>
#include <QByteArray>
#include "settings.h"

class Transport : public QObject
{
    Q_OBJECT
public:
    explicit Transport(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~Transport() = default;

    virtual bool open(const SerialSettings &settings) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
    virtual void send(const QByteArray &data) = 0;

signals:
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &err);
};

#endif // TRANSPORT_H
