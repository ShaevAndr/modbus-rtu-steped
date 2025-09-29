#ifndef SEARCHDEVICEWIDGET_H
#define SEARCHDEVICEWIDGET_H

#include <QDialog>
#include <QVector>

namespace Ui {
class SearchDeviceWidget;
}

class SearchDeviceWidget : public QDialog
{
    Q_OBJECT

public:
    struct Device {
        int address;
        QString description;
    };

    explicit SearchDeviceWidget(QWidget *parent = nullptr);
    ~SearchDeviceWidget();

signals:
    void cancelSearch();
    void broadcastSearch();
    void checkAddress(int address);

private:
    Ui::SearchDeviceWidget *ui;
    int startAddressSearch;
    int endAddressSearch;
    bool isBroadcast;

    void handleSearchButtonClick();
    void handleBroadcastSelect(bool checkboxValue);

private slots:
    void handleCheckResult(bool hasDevice);
    void handleCheckResult(bool hasDevice, const Device& device);
    void handleCheckResult(const QVector<Device>& devices);


};

#endif // SEARCHDEVICEWIDGET_H
