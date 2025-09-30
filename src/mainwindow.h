#pragma once
#include <QMainWindow>
#include <QSerialPort>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QVector>
#include <QThread>
#include "settings.h"
#include "request.h"
#include "device.h"
#include "searchdevicewidget.h"

// Forward declarations
class Transport;
class IProtocol;
class Master;
struct Response;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    virtual void getParametrsInt(quint16 startRegistr, quint16 paramsCount);
    virtual void getParametrsFloat(quint16 startRegistr, quint16 paramsCount);

private slots:
    void refreshPorts();
    void onConnectClicked();
    void handleGetParametrIntButtonClick();
    void handleGetParametrsIntButtonClick();
    void handleGetParametrFloatButtonClick();
    void handleGetParametrsFloatButtonClick();
    void handleFindDevicesButtonClick();
    void onDeviceSelected(int address);
    void checkDeviceAddress(int address);
    void broadcastSearchDevices();

signals:
    void checkDeviceResult(bool result, int address);
    void checkDeviceResult(bool result, int address, const QString& description);
    void broadcastSerchDeviceResult(const QMap<int, QString>& devices);

private:
    void setupUi();
    void updateUiConnected(bool connected);

    // UI
    QComboBox *portCombo;
    QComboBox *baudCombo;
    QComboBox *parityCombo;
    QComboBox *dataBitsCombo;
    QComboBox *stopBitsCombo;
    QPushButton *refreshButton;
    QPushButton *connectButton;

    QLineEdit *addrEdit;
    QLineEdit *startRegistrEdit;
    QLineEdit *funcEdit;

    QLineEdit *dataEdit;
    QLabel *crcLabel;
    QPushButton *getIntButton;
    QPushButton *getIntsButton;
    QPushButton *getFloatButton;
    QPushButton *getFloatsButton;
    QPushButton *setIntButton;
    QPushButton *setIntsButton;
    QPushButton *setFloatButton;
    QPushButton *setFloatsButton;
    QPushButton *findDevices;


    QTextEdit *logEdit;

    SearchDeviceWidget *searchWidget;

    // backend
    Transport* m_transport;
    IProtocol* m_protocol;
    Master* m_master;
    SerialSettings settings;
    QVector<Device> m_devices;
};
