#ifndef MASTER_H
#define MASTER_H

#include <QObject>
#include <QQueue>
#include <QVector>
#include <Qtimer>
#include "transport.h"
#include "IProtocol.h"
#include "command.h"
#include "request.h"

class Master : public QObject
{
    Q_OBJECT
public:
    explicit Master(Transport *transport, IProtocol *protocol, QObject *parent = nullptr);
    ~Master();

    // Метод для добавления задачи в очередь
    void enqueueJob(const Request &request);

private slots:
    void onDataReceived(const QByteArray &data);
    void onErrorOccurred(const QString &error);
    void processQueue();
    void onTimeout();

private:
    void sendCommand(const Command &cmd);

    Transport *m_transport;
    IProtocol *m_protocol;

    // Очередь "заданий", где каждое задание - это вектор команд
    QQueue<Request> m_jobQueue;
    Request m_currentJob;
    QVector<Response> m_currentResponses;
    int m_currentCommandIndex = 0;
    QTimer m_timer;

signals:
    // Signals are no longer needed, callbacks are used instead
};

#endif // MASTER_H
