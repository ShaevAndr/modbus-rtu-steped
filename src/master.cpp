#include "master.h"
#include <QDebug>

Master::Master(Transport *transport, IProtocol *protocol, QObject *parent)
    : QObject(parent), m_transport(transport), m_protocol(protocol)
{
    Q_ASSERT(m_transport != nullptr);
    Q_ASSERT(m_protocol != nullptr);

    connect(m_transport, &Transport::dataReceived, this, &Master::onDataReceived);
    connect(m_transport, &Transport::errorOccurred, this, &Master::onErrorOccurred);
}

Master::~Master()
{
}

void Master::enqueueJob(const QVector<Command> &commands)
{
    if (commands.isEmpty()) {
        return;
    }
    m_jobQueue.enqueue(commands);

    // Если очередь была пуста, запускаем обработку
    if (m_jobQueue.size() == 1 && m_currentJob.isEmpty()) {
        processQueue();
    }
}

void Master::onDataReceived(const QByteArray &data)
{
    qDebug() << "Master received data:" << data.toHex(' ');

    if (m_currentJob.isEmpty()) {
        qWarning() << "Received data without an active job.";
        return;
    }

    // Декодируем ответ
    Response response = m_protocol->decode(data);

    if (response.errorCode != ecNoError) {
        // Обработка ошибки протокола (CRC, etc.)
        onErrorOccurred("Protocol error: " + response.errorMessage);
        // TODO: здесь можно добавить логику повтора команды или отмены всего задания
        return;
    }

    // --- Здесь будет логика обработки ответа ---
    // Например, если это была "подготовительная" команда,
    // на основе response.data можно сформировать и добавить новые команды в m_currentJob
    qDebug() << "Response successfully decoded.";
    m_currentResponses.append(response);


    // Переходим к следующей команде в текущем задании
    m_currentCommandIndex++;
    if (m_currentCommandIndex < m_currentJob.size()) {
        sendCommand(m_currentJob[m_currentCommandIndex]);
    } else {
        // Задание выполнено, переходим к следующему в очереди
        qDebug() << "Job finished.";
        emit jobFinished(m_currentResponses);
        m_currentJob.clear();
        m_currentCommandIndex = 0;
        m_currentResponses.clear();
        processQueue();
    }
}

void Master::onErrorOccurred(const QString &error)
{
    qWarning() << "Master received error:" << error;
    emit jobError(error);

    // Очищаем текущее задание и пытаемся запустить следующее из очереди
    m_currentJob.clear();
    m_currentResponses.clear();
    m_currentCommandIndex = 0;
    processQueue();
}

void Master::processQueue()
{
    if (m_jobQueue.isEmpty() || !m_currentJob.isEmpty()) {
        return;
    }

    m_currentJob = m_jobQueue.dequeue();
    m_currentCommandIndex = 0;
    m_currentResponses.clear();

    if (!m_currentJob.isEmpty()) {
        qDebug() << "Starting new job.";
        sendCommand(m_currentJob[m_currentCommandIndex]);
    }
}

void Master::sendCommand(const Command &cmd)
{
    if (!m_transport->isOpen()) {
        onErrorOccurred("Transport is not open.");
        return;
    }

    QByteArray request = m_protocol->encode(cmd);
    qDebug() << "Master sending data:" << request.toHex(' ');
    m_transport->send(request);
}