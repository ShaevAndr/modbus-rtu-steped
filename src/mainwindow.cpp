#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QRegExp>
#include "modbuscrc.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), io(new IRxTx(this))
{
    setupUi();

    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshPorts);
    connect(connectButton, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendClicked);

    connect(io, &IRxTx::dataReceived, this, &MainWindow::onDataReceived);
    connect(io, &IRxTx::errorOccurred, this, &MainWindow::onError);

    refreshPorts();
}

MainWindow::~MainWindow()
{
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
    dataBitsCombo->addItems({"5","6","7","8"});
    stopBitsCombo->addItems({"1","1.5","2"});

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
    setWindowTitle("Modbus RTU - Qt Example");
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
    if (io->isOpen()) {
        io->close();
        updateUiConnected(false);
        logEdit->append("Disconnected");
        return;
    }

    settings.portName = portCombo->currentText();
    settings.baudRate = baudCombo->currentText().toInt();
    settings.parity = static_cast<QSerialPort::Parity>(parityCombo->currentIndex());
    settings.dataBits = static_cast<QSerialPort::DataBits>(dataBitsCombo->currentText().toInt());
    settings.stopBits = (stopBitsCombo->currentText() == "1") ? QSerialPort::OneStop : QSerialPort::TwoStop;

    bool ok = io->open(settings);
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
    if (!io->isOpen()) {
        QMessageBox::warning(this, "Not connected", "Open serial port first");
        return;
    }

    bool ok;
    int addr = addrEdit->text().toInt(&ok);
    if (!ok) { QMessageBox::warning(this, "Input error", "Invalid address"); return; }
    int func = funcEdit->text().toInt(&ok);
    if (!ok) { QMessageBox::warning(this, "Input error", "Invalid function"); return; }

    QByteArray data = hexStringToBytes(dataEdit->text());
    QByteArray frame;
    frame.append(static_cast<char>(addr & 0xFF));
    frame.append(static_cast<char>(func & 0xFF));
    frame.append(data);

    quint16 crc = ModbusCRC::calculate(reinterpret_cast<const unsigned char*>(frame.constData()), frame.size());
    frame.append(static_cast<char>(crc & 0xFF));
    frame.append(static_cast<char>((crc >> 8) & 0xFF));

    crcLabel->setText(QString("CRC: 0x%1").arg(QString::number(crc, 16).toUpper()));

    io->send(frame);
    logEdit->append(QString("Sent: %1").arg(QString(frame.toHex(' ').toUpper())));
}

void MainWindow::onDataReceived(const QByteArray &data)
{
    logEdit->append(QString("Received: %1").arg(QString(data.toHex(' ').toUpper())));
}

void MainWindow::onError(const QString &err)
{
    logEdit->append(QString("Error: %1").arg(err));
}
