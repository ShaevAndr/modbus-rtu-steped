#pragma once
#include <QMainWindow>
#include <QSerialPort>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include "settings.h"
#include "request.h"
#include "devicegridwidget.h"

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
    virtual void checkDevice(quint8 deviceAddress);

private slots:
    void refreshPorts();
    void onConnectClicked();
    void handleGetParametrIntButtonClick();
    void handleGetParametrsIntButtonClick();
    void handleGetParametrFloatButtonClick();
    void handleGetParametrsFloatButtonClick();
    void handleFindDevicesButtonClick();

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

    // backend
    Transport* m_transport;
    IProtocol* m_protocol;
    Master* m_master;
    SerialSettings settings;
    DeviceGridWidget *m_searchWidget;
};
