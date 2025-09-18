#ifndef MASTER_H
#define MASTER_H

#include <QObject>
#include <QQueue>
#include <QVector>
#include "transport.h"
#include "IProtocol.h"
#include "command.h"

class Master : public QObject
{
    Q_OBJECT
public:
    explicit Master(Transport *transport, IProtocol *protocol, QObject *parent = nullptr);
    ~Master();

    // Метод для добавления задачи в очередь
    void enqueueJob(const QVector<Command> &commands);

private slots:
    void onDataReceived(const QByteArray &data);
    void onErrorOccurred(const QString &error);
    void processQueue();

private:
    void sendCommand(const Command &cmd);

    Transport *m_transport;
    IProtocol *m_protocol;

    // Очередь "заданий", где каждое задание - это вектор команд
    QQueue<QVector<Command>> m_jobQueue;
    QVector<Command> m_currentJob;
    QVector<Response> m_currentResponses;
    int m_currentCommandIndex = 0;

signals:
    void jobFinished(const QVector<Response> &responses);
    void jobError(const QString &error);
};

#endif // MASTER_H