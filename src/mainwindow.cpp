#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QRegExp>

#include "serialporttransport.h"
#include "modbusrtuprotocol.h"
#include "master.h"
#include "command.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    // Инициализация бэкенда
    m_transport = new SerialPortTransport(this);
    m_protocol = new ModbusRtuProtocol(); // Не QObject, родитель не нужен
    m_master = new Master(m_transport, m_protocol, this);

    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshPorts);
    connect(connectButton, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendClicked);

    // Соединяем сигналы от Master со слотами GUI
    connect(m_master, &Master::jobFinished, this, &MainWindow::onJobFinished);
    connect(m_master, &Master::jobError, this, &MainWindow::onJobError);

    refreshPorts();
}

MainWindow::~MainWindow()
{
    delete m_protocol; // Удаляем, т.к. у него нет родителя
}

void MainWindow::setupUi()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    portCombo = new QComboBox;
    baudCombo = new QComboBox;
    parityCombo = new QComboBox;
    dataBitsCombo = new QComboBox;
    stopBitsCombo = new QComboBox;

    refreshButton = new QPushButton("Refresh");
    connectButton = new QPushButton("Connect");

    addrEdit = new QLineEdit; addrEdit->setPlaceholderText("slave addr (e.g. 1)");
    funcEdit = new QLineEdit; funcEdit->setPlaceholderText("function (e.g. 3)");
    dataEdit = new QLineEdit; dataEdit->setPlaceholderText("data bytes hex (e.g. 00 10 00 02)");
    crcLabel = new QLabel("CRC: --");
    sendButton = new QPushButton("Send");

    logEdit = new QTextEdit; logEdit->setReadOnly(true);

    // Fill combos
    baudCombo->addItems({"9600","19200","38400","57600","115200"});
    parityCombo->addItems({"None","Even","Odd"});
    dataBitsCombo->addItems({"8","7","6","5"});
    stopBitsCombo->addItems({"1","2"}); // 1.5 не поддерживается QSerialPort

    QFormLayout *form = new QFormLayout;
    QHBoxLayout *portRow = new QHBoxLayout;
    portRow->addWidget(portCombo);
    portRow->addWidget(refreshButton);
    portRow->addWidget(connectButton);

    QWidget *portWidget = new QWidget;
    portWidget->setLayout(portRow);
    form->addRow("Port:", portWidget);
    form->addRow("Baud:", baudCombo);
    form->addRow("Parity:", parityCombo);
    form->addRow("Data bits:", dataBitsCombo);
    form->addRow("Stop bits:", stopBitsCombo);

    QHBoxLayout *cmdRow = new QHBoxLayout;
    cmdRow->addWidget(addrEdit);
    cmdRow->addWidget(funcEdit);
    cmdRow->addWidget(dataEdit);
    cmdRow->addWidget(crcLabel);
    cmdRow->addWidget(sendButton);

    QVBoxLayout *mainLay = new QVBoxLayout;
    mainLay->addLayout(form);
    mainLay->addLayout(cmdRow);
    mainLay->addWidget(new QLabel("Response / Log:"));
    mainLay->addWidget(logEdit);

    central->setLayout(mainLay);
    setWindowTitle("Modbus RTU Master");
}

void MainWindow::refreshPorts()
{
    portCombo->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &p : ports) {
        portCombo->addItem(p.portName());
    }
}

void MainWindow::onConnectClicked()
{
    if (m_transport->isOpen()) {
        m_transport->close();
        updateUiConnected(false);
        logEdit->append("Disconnected");
        return;
    }

    settings.portName = portCombo->currentText();
    settings.baudRate = baudCombo->currentText().toInt();
    settings.parity = static_cast<QSerialPort::Parity>(parityCombo->currentIndex());
    settings.dataBits = static_cast<QSerialPort::DataBits>(dataBitsCombo->currentText().toInt());
    settings.stopBits = (stopBitsCombo->currentText() == "1") ? QSerialPort::OneStop : QSerialPort::TwoStop;

    bool ok = m_transport->open(settings);
    if (!ok) {
        QMessageBox::critical(this, "Error", "Failed to open port");
        logEdit->append("Failed to open port");
    } else {
        updateUiConnected(true);
        logEdit->append(QString("Connected to %1").arg(settings.portName));
    }
}

void MainWindow::updateUiConnected(bool connected)
{
    connectButton->setText(connected ? "Disconnect" : "Connect");
    portCombo->setEnabled(!connected);
    baudCombo->setEnabled(!connected);
    parityCombo->setEnabled(!connected);
    dataBitsCombo->setEnabled(!connected);
    stopBitsCombo->setEnabled(!connected);
    refreshButton->setEnabled(!connected);
}

static QByteArray hexStringToBytes(const QString &s)
{
    QByteArray out;
    QStringList parts = s.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    for (const QString &p : parts) {
        bool ok;
        int val = p.toInt(&ok, 16);
        if (ok) out.append(static_cast<char>(val & 0xFF));
    }
    return out;
}

void MainWindow::onSendClicked()
{
    if (!m_transport->isOpen()) {
        QMessageBox::warning(this, "Not connected", "Open serial port first");
        return;
    }

    bool ok;
    int addr = addrEdit->text().toInt(&ok);
    if (!ok || addr < 0 || addr > 255) { QMessageBox::warning(this, "Input error", "Invalid address"); return; }
    int func = funcEdit->text().toInt(&ok);
    if (!ok || func < 0 || func > 255) { QMessageBox::warning(this, "Input error", "Invalid function"); return; }

    QByteArray data = hexStringToBytes(dataEdit->text());

    // Создаем команду и задание
    Command cmd(addr, func, data);
    QVector<Command> job;
    job.append(cmd);

    // Отправляем задание в Master
    m_master->enqueueJob(job);

    // Отображаем, что мы отправили (без CRC, т.к. его добавит протокол)
    QByteArray frame;
    frame.append(cmd.deviceAddress);
    frame.append(cmd.functionCode);
    frame.append(cmd.data);
    logEdit->append(QString("Sent job with 1 command: %1").arg(QString(frame.toHex(' ').toUpper())));
}

void MainWindow::onJobFinished(const QVector<Response> &responses)
{
    for(const Response& resp : responses) {
        logEdit->append(QString("Received: %1").arg(QString(resp.data.toHex(' ').toUpper())));
    }
}

void MainWindow::onJobError(const QString &err)
{
    logEdit->append(QString("<font color='red'>Error: %1</font>").arg(err));
}