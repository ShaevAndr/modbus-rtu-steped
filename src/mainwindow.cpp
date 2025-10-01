#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QRegExp>
#include <QDebug>

#include "serialporttransport.h"
#include "modbusrtuprotocol.h"
#include "master.h"
#include "command.h"
#include "devicescandialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    // Инициализация бэкенда
    m_transport = new SerialPortTransport(this);
    m_protocol = new ModbusRtuProtocol(); // Не QObject, родитель не нужен
    m_master = new Master(m_transport, m_protocol, this);

    QThread *transportTrhead = new QThread;

    m_transport->moveToThread(transportTrhead);

    connect(m_master, &Master::send, m_transport, &Transport::send, Qt::QueuedConnection);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshPorts);
    connect(connectButton, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(getIntButton, &QPushButton::clicked, this, &MainWindow::handleGetParametrIntButtonClick);
    connect(getIntsButton, &QPushButton::clicked, this, &MainWindow::handleGetParametrsIntButtonClick);
    connect(getFloatButton, &QPushButton::clicked, this, &MainWindow::handleGetParametrFloatButtonClick);
    connect(getFloatsButton, &QPushButton::clicked, this, &MainWindow::handleGetParametrsFloatButtonClick);
    // connect(findDevices, &QPushButton::clicked, this, &MainWindow::handleFindDevicesButtonClick);
    connect(findDevices, &QPushButton::clicked, searchWidget, [this](){
        searchWidget->show();
    });
    connect(searchWidget, &SearchDeviceWidget::checkAddress, this, &MainWindow::checkDeviceAddress);
    connect(searchWidget, &SearchDeviceWidget::broadcastSearch, this, &MainWindow::broadcastSearchDevices);
    connect(this,
            QOverload<bool,int>::of(&MainWindow::checkDeviceResult),
            searchWidget,
            QOverload<bool,int>::of(&SearchDeviceWidget::handleCheckResult));

    transportTrhead->start();
    connect(this,
            QOverload<bool,int, const QString&>::of(&MainWindow::checkDeviceResult),
            searchWidget,
            QOverload<bool,int, const QString&>::of(&SearchDeviceWidget::handleCheckResult));

    transportTrhead->start();

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

    searchWidget = new SearchDeviceWidget;
    portCombo = new QComboBox;
    baudCombo = new QComboBox;
    parityCombo = new QComboBox;
    dataBitsCombo = new QComboBox;
    stopBitsCombo = new QComboBox;

    refreshButton = new QPushButton("Refresh");
    connectButton = new QPushButton("Connect");

    addrEdit = new QLineEdit; addrEdit->setPlaceholderText("slave addr (e.g. 1)");
    startRegistrEdit = new QLineEdit; startRegistrEdit->setPlaceholderText("registr bytes hex (e.g. 00 10 00 02)");
    dataEdit = new QLineEdit; dataEdit->setPlaceholderText("data bytes hex (e.g. 00 10 00 02)");
    crcLabel = new QLabel("CRC: --");
    getIntButton = new QPushButton("getInt");
    getIntsButton = new QPushButton("getInts");
    getFloatButton = new QPushButton("getFloat");
    getFloatsButton = new QPushButton("getFloats");
    setIntButton = new QPushButton("getInt");
    setIntsButton = new QPushButton("getInts");
    setFloatButton = new QPushButton("getFloat");
    setFloatsButton = new QPushButton("getFloats");
    findDevices = new QPushButton("findDevices");

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
    cmdRow->addWidget(startRegistrEdit);
    cmdRow->addWidget(dataEdit);


    cmdRow->addWidget(crcLabel);
    cmdRow->addWidget(getIntButton);
    cmdRow->addWidget(getFloatButton);
    cmdRow->addWidget(getIntsButton);
    cmdRow->addWidget(getFloatsButton);
    cmdRow->addWidget(setIntButton);
    cmdRow->addWidget(setFloatButton);
    cmdRow->addWidget(setIntsButton);
    cmdRow->addWidget(setFloatsButton);
    cmdRow->addWidget(findDevices);

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

void MainWindow::checkDeviceAddress(int address) {
    qDebug()<<"проверка устройства по адресу - "<< address;
    if (address==2) {
        emit checkDeviceResult(true, address, "new Device");
    }else {
        emit checkDeviceResult(false, address);
    }
}

void MainWindow::broadcastSearchDevices() {
    qDebug()<<"широкополосный поиск устройств";
    QMap<int, QString> result = {{2, "new device"}, {3, "new device"}, {23, "new device"}, {24, "new device"}};
    emit broadcastSerchDeviceResult(result);
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

void MainWindow::handleGetParametrIntButtonClick() {
    bool ok;

    quint16 registr = static_cast<quint16>(startRegistrEdit->text().toInt(&ok));
    if (!ok) { QMessageBox::warning(this, "Input error", "Invalid registr address"); return; }
    MainWindow::getParametrsInt(registr, 1);
}

void MainWindow::handleGetParametrsIntButtonClick() {
    bool ok;
    quint16 registr = static_cast<quint16>(startRegistrEdit->text().toInt(&ok));
    if (!ok) { QMessageBox::warning(this, "Input error", "Invalid registr address"); return; }
    quint16 count = static_cast<quint16>(dataEdit->text().toInt(&ok));
    if (!ok) { QMessageBox::warning(this, "Input error", "Invalid registr count"); return; }
    MainWindow::getParametrsInt(registr, count);
}

void MainWindow::handleGetParametrFloatButtonClick() {
    bool ok;
    quint16 registr = static_cast<quint16>(startRegistrEdit->text().toInt(&ok));
    if (!ok) { QMessageBox::warning(this, "Input error", "Invalid registr address"); return; }
    MainWindow::getParametrsFloat(registr, 1);
}

void MainWindow::handleGetParametrsFloatButtonClick() {
    bool ok;
    quint16 registr = static_cast<quint16>(startRegistrEdit->text().toInt(&ok));
    if (!ok) { QMessageBox::warning(this, "Input error", "Invalid registr address"); return; }
    quint16 count = static_cast<quint16>(dataEdit->text().toInt(&ok));
    if (!ok) { QMessageBox::warning(this, "Input error", "Invalid registr count"); return; }
    MainWindow::getParametrsFloat(registr, count);
}

void MainWindow::handleFindDevicesButtonClick() {
    if (!m_transport->isOpen()){
        QMessageBox::warning(this, "Not connected", "Open serial port first");
        return;
    }

    DeviceScanDialog dialog(m_master, m_protocol, this);
    connect(&dialog, &DeviceScanDialog::deviceSelected, this, &MainWindow::onDeviceSelected);
    dialog.exec();
}

void MainWindow::onDeviceSelected(int address)
{
    Device newDevice;
    newDevice.address = address;
    m_devices.append(newDevice);
    logEdit->append(QString("Device %1 added to the list of active devices.").arg(address));
}

void MainWindow::getParametrsInt(quint16 startRegistr, quint16 paramsCount) {
    if (!m_transport->isOpen()) {
        QMessageBox::warning(this, "Not connected", "Open serial port first");
        return;
    }

    Request request;
    request.commands = m_protocol->getParametersI(1, startRegistr, paramsCount);
    request.onSuccess = [this](const QVector<Response> &responses){
        for(const Response& resp : responses) {
            logEdit->append(QString("Received: %1").arg(QString(resp.data.toHex(' ').toUpper())));
        }
    };
    request.onError = [this](const QString &err){
        logEdit->append(QString("<font color='red'>Error: %1</font>").arg(err));
    };

    m_master->enqueueJob(request);

    for (const Command& cmd: request.commands) {
         logEdit->append(m_protocol->encode(cmd).toHex());
    }
}


void MainWindow::getParametrsFloat(quint16 startRegistr, quint16 paramsCount) {
    if (!m_transport->isOpen()) {
        QMessageBox::warning(this, "Not connected", "Open serial port first");
        return;
    }
    
    Request request;
    request.commands = m_protocol->getParametersF(1, startRegistr, paramsCount);
    request.onSuccess = [this](const QVector<Response> &responses){
        for(const Response& resp : responses) {
            logEdit->append(QString("Received: %1").arg(QString(resp.data.toHex(' ').toUpper())));
        }
    };
    request.onError = [this](const QString &err){
        logEdit->append(QString("<font color='red'>Error: %1</font>").arg(err));
    };

    m_master->enqueueJob(request);

    for (const Command& cmd: request.commands) {
         logEdit->append(m_protocol->encode(cmd).toHex());
    }
}
