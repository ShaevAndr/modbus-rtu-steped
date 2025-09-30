#include "searchdevicewidget.h"
#include "ui_searchdevicewidget.h"

SearchDeviceWidget::SearchDeviceWidget(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SearchDeviceWidget)
{
    ui->setupUi(this);
    connect(ui->CancelSearchButton, &QPushButton::clicked, this, &SearchDeviceWidget::cancelSearch);
    connect(ui->checkBox, &QCheckBox::clicked, this, &SearchDeviceWidget::handleBroadcastSelect);
}



SearchDeviceWidget::~SearchDeviceWidget()
{
    delete ui;
}

void SearchDeviceWidget::handleBroadcastSelect(bool checkboxValue)
{
    qDebug()<<checkboxValue;
    if (checkboxValue) {
        ui->IntervalFrom->setEnabled(false);
        ui->IntervalTo->setEnabled(false);
    } else {
        ui->IntervalFrom->setEnabled(true);
        ui->IntervalTo->setEnabled(true);
    }
}

void SearchDeviceWidget::handleCheckResult(bool hasDevice, int address)
{
    qDebug()<<hasDevice<<"-"<<address;
    return;
}

void SearchDeviceWidget::handleCheckResult(bool hasDevice, int address, const QString& description)
{
    qDebug()<<hasDevice<<"-"<<address<<"-"<<description;
    return;
}

void SearchDeviceWidget::handleCheckResult(const QMap<int, QString>& devices)
{
    for (auto it = devices.cbegin(); it != devices.cend(); ++it) {
        qDebug() << "key:" << it.key() << "value:" << it.value();
    }
    return;
}


