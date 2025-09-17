#pragma once
#include <QMainWindow>
#include <QSerialPort>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include "irxtx.h"
#include "settings.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void refreshPorts();
    void onConnectClicked();
    void onSendClicked();
    void onDataReceived(const QByteArray &data);
    void onError(const QString &err);

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
    QLineEdit *funcEdit;
    QLineEdit *dataEdit;
    QLabel *crcLabel;
    QPushButton *sendButton;

    QTextEdit *logEdit;

    // backend
    IRxTx *io;
    SerialSettings settings;
};
