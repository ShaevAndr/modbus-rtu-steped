#include "devicescandialog.h"
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QEvent>
#include <QDebug>
#include <QScrollArea>

#include "master.h"
#include "IProtocol.h"
#include "request.h"

DeviceScanDialog::DeviceScanDialog(Master* master, IProtocol* protocol, QWidget *parent) :
    QDialog(parent),
    m_master(master),
    m_protocol(protocol)
{
    setupUi();
    connect(m_scanButton, &QPushButton::clicked, this, &DeviceScanDialog::onScanButtonClicked);
}

DeviceScanDialog::~DeviceScanDialog()
{
}

void DeviceScanDialog::setupUi()
{
    setWindowTitle("Device Scan");
    setMinimumSize(800, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    m_scanButton = new QPushButton("Scan");
    mainLayout->addWidget(m_scanButton);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    QWidget *gridContainer = new QWidget();
    m_grid = new QGridLayout(gridContainer);
    m_grid->setSpacing(5);

    for (int i = 1; i <= 247; ++i) {
        QLabel *label = new QLabel(QString::number(i));
        label->setAlignment(Qt::AlignCenter);
        label->setFrameShape(QFrame::Box);
        label->setMinimumSize(40, 40);
        label->setProperty("address", i);
        label->installEventFilter(this);

        m_deviceLabels.insert(i, label);
        m_deviceStatus.insert(i, 0);
        setDeviceStatus(i, 0); // Set initial gray color

        int row = (i - 1) / 16;
        int col = (i - 1) % 16;
        m_grid->addWidget(label, row, col);
    }
    
    scrollArea->setWidget(gridContainer);
    mainLayout->addWidget(scrollAria);

    setLayout(mainLayout);
}

void DeviceScanDialog::onScanButtonClicked()
{
    if (!m_master->isTransportOpen()) {
        QMessageBox::warning(this, "Not connected", "Open serial port first");
        return;
    }

    // Reset all to gray before scan
    for (int i = 1; i <= 247; ++i) {
        setDeviceStatus(i, 0);
    }

    for (quint8 i = 1; i < 248; i++) {
        checkDevice(i);
    }
}

void DeviceScanDialog::checkDevice(quint8 deviceAddress)
{
    Request request;
    request.commands = m_protocol->getParametersI(deviceAddress, 1, 1); // Simple request

    request.onSuccess = [this, deviceAddress](const QVector<Response> &) {
        setDeviceStatus(deviceAddress, 1); // Found
    };
    request.onError = [this, deviceAddress](const QString &) {
        setDeviceStatus(deviceAddress, -1); // Not found
    };

    m_master->enqueueJob(request);
}

void DeviceScanDialog::setDeviceStatus(int address, int status)
{
    QLabel *label = m_deviceLabels.value(address);
    if (!label) return;

    m_deviceStatus[address] = status;
    QString styleSheet;
    switch (status) {
    case 1: // Found
        styleSheet = "background-color: green; color: white;";
        break;
    case -1: // Not Found
        styleSheet = "background-color: red; color: white;";
        break;
    case 0: // Neutral / Waiting
    default:
        styleSheet = "background-color: gray; color: white;";
        break;
    }
    label->setStyleSheet(styleSheet);
}

bool DeviceScanDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QLabel *label = qobject_cast<QLabel*>(watched);
        if (label) {
            int address = label->property("address").toInt();
            if (m_deviceStatus.value(address) == 1) { // If found (green)
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(this, "Connect Device",
                                              QString("Do you want to connect to device with address %1?").arg(address),
                                              QMessageBox::Yes|QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                    emit deviceSelected(address);
                    accept();
                }
            }
            return true; // Event handled
        }
    }
    return QObject::eventFilter(watched, event);
}
