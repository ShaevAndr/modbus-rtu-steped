#include "searchdevicewidget.h"
#include "ui_searchdevicewidget.h"

SearchDeviceWidget::SearchDeviceWidget(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SearchDeviceWidget)
{
    ui->setupUi(this);
    connect(ui->CancelSearchButton, &QPushButton::clicked, this, &SearchDeviceWidget::cancelSearch);
}



SearchDeviceWidget::~SearchDeviceWidget()
{
    delete ui;
}

void SearchDeviceWidget::handleBroadcastSelect(bool checkboxValue)
{
    if (checkboxValue) {
        ui->IntervalFrom->setEnabled(false);
        ui->IntervalTo->setEnabled(false);
    } else {
        ui->IntervalFrom->setEnabled(true);
        ui->IntervalTo->setEnabled(true);
    }
}

void SearchDeviceWidget::handleCheckResult(bool hasDevice)
{
    return;
}

void SearchDeviceWidget::handleCheckResult(bool hasDevice, const Device &device)
{
    return;
}

void SearchDeviceWidget::handleCheckResult(const QVector<Device> &devices)
{
    return;
}


