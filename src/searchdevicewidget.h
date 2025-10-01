#ifndef SEARCHDEVICEWIDGET_H
#define SEARCHDEVICEWIDGET_H

#include <QDialog>
#include <QVector>
#include <QDebug>
#include <QList>

namespace Ui {
class SearchDeviceWidget;
}

class SearchDeviceWidget : public QDialog
{
    Q_OBJECT

public:
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
    QList<int> *addressesList;

    void handleSearchButtonClick();
    void handleBroadcastSelect(bool checkboxValue);
    void processCheckAddress();

public slots:
    void handleCheckResult(bool hasDevice, int address);
    void handleCheckResult(bool hasDevice, int address, const QString& description);
    void handleCheckResult(const QMap<int, QString>& devices);

};

#endif // SEARCHDEVICEWIDGET_H
