#include "master.h"
#include <QDebug>

Master::Master(Transport *transport, IProtocol *protocol, QObject *parent)
    : QObject(parent), m_transport(transport), m_protocol(protocol)
{
    Q_ASSERT(m_transport != nullptr);
    Q_ASSERT(m_protocol != nullptr);

    connect(m_transport, &Transport::dataReceived, this, &Master::onDataReceived);
    connect(m_transport, &Transport::errorOccurred, this, &Master::onErrorOccurred);

    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &Master::onTimeout);
}

bool Master::isTransportOpen() const
{
    return m_transport->isOpen();
}

Master::~Master()
{
}

void Master::enqueueJob(const Request &request)
{
    if (request.commands.isEmpty()) {
        return;
    }
    m_jobQueue.enqueue(request);

    // Если очередь была пуста, запускаем обработку
    if (m_jobQueue.size() == 1 && m_currentJob.commands.isEmpty()) {
        processQueue();
    }
}

void Master::onDataReceived(const QByteArray &data)
{
    m_timer.stop(); // <-- Сброс таймера, так как ответ пришёл
    qDebug() << "Master received data:" << data.toHex(' ');

    if (m_currentJob.commands.isEmpty()) {
        qWarning() << "Received data without an active job.";
        return;
    }

    // Декодируем ответ
    Response response = m_protocol->decode(data);

    if (response.errorCode != ecNoError) {
        // Обработка ошибки протокола (CRC, etc.)
        onErrorOccurred("Protocol error: " + response.errorMessage);
        return;
    }

    qDebug() << "Response successfully decoded.";
    m_currentResponses.append(response);


    // Переходим к следующей команде в текущем задании
    m_currentCommandIndex++;
    if (m_currentCommandIndex < m_currentJob.commands.size()) {
        sendCommand(m_currentJob.commands[m_currentCommandIndex]);
    } else {
        // Задание выполнено, вызываем колбэк и переходим к следующему
        qDebug() << "Job finished.";
        if (m_currentJob.onSuccess) {
            m_currentJob.onSuccess(m_currentResponses);
        }
        m_currentJob = Request(); // Clear current job
        m_currentCommandIndex = 0;
        m_currentResponses.clear();
        processQueue();
    }
}

void Master::onErrorOccurred(const QString &error)
{
    qWarning() << "Master received error:" << error;
    if (m_currentJob.onError) {
        m_currentJob.onError(error);
    }

    // Очищаем текущее задание и пытаемся запустить следующее из очереди
    m_currentJob = Request(); // Clear current job
    m_currentResponses.clear();
    m_currentCommandIndex = 0;
    processQueue();
}

void Master::processQueue()
{
    if (m_jobQueue.isEmpty() || !m_currentJob.commands.isEmpty()) {
        return;
    }

    m_currentJob = m_jobQueue.dequeue();
    m_currentCommandIndex = 0;
    m_currentResponses.clear();

    if (!m_currentJob.commands.isEmpty()) {
        qDebug() << "Starting new job.";
        sendCommand(m_currentJob.commands[m_currentCommandIndex]);
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

    m_timer.start(1000);
}

void Master::onTimeout()
{
    qWarning() << "Timeout waiting for response.";

    if (m_currentJob.onError) {
        m_currentJob.onError("Timeout waiting for response");
    }

    // Сброс текущего задания и переход к следующему
    m_currentJob = Request();
    m_currentResponses.clear();
    m_currentCommandIndex = 0;

    processQueue();
}
