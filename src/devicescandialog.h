#ifndef DEVICESCANDIALOG_H
#define DEVICESCANDIALOG_H

#include <QDialog>
#include <QMap>

class QGridLayout;
class QPushButton;
class QLabel;
class Master;
class IProtocol;

class DeviceScanDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceScanDialog(Master* master, IProtocol* protocol, QWidget *parent = nullptr);
    ~DeviceScanDialog();

signals:
    void deviceSelected(int address);

private slots:
    void onScanButtonClicked();

private:
    void setupUi();
    void checkDevice(quint8 deviceAddress);
    void setDeviceStatus(int address, int status); // 0: neutral, 1: found, -1: not found
    bool eventFilter(QObject *watched, QEvent *event) override;


    Master* m_master;
    IProtocol* m_protocol;

    QGridLayout *m_grid;
    QPushButton *m_scanButton;
    QMap<int, QLabel*> m_deviceLabels;
    QMap<int, int> m_deviceStatus; // To track status and avoid re-clicking
};

#endif // DEVICESCANDIALOG_H
