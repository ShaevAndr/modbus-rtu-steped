#pragma once
#include <QMainWindow>
#include <QSerialPort>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include "settings.h"

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

private slots:
    void refreshPorts();
    void onConnectClicked();
    void onGetInt();
    void onJobFinished(const QVector<Response> &responses);
    void onJobError(const QString &err);

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


    QTextEdit *logEdit;

    // backend
    Transport* m_transport;
    IProtocol* m_protocol;
    Master* m_master;
    SerialSettings settings;
};
