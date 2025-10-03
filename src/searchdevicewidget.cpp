#include <QIntValidator>
#include "searchdevicewidget.h"
#include "ui_searchdevicewidget.h"
#include "ConnectDeviceButton.cpp"

SearchDeviceWidget::SearchDeviceWidget(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SearchDeviceWidget)
{
    ui->setupUi(this);
    addressesList = new QList<int>;
    QIntValidator *intervalValidator = new QIntValidator(0, 247, this);
    ui->IntervalFrom->setValidator(intervalValidator);
    ui->IntervalTo->setValidator(intervalValidator);

    connect(ui->CancelSearchButton, &QPushButton::clicked, this, &SearchDeviceWidget::cancelSearch);
    connect(ui->checkBox, &QCheckBox::clicked, this, &SearchDeviceWidget::handleBroadcastSelect);
    connect(ui->SearchDeviceButton, &QPushButton::clicked, this, &SearchDeviceWidget::handleSearchButtonClick);
}

SearchDeviceWidget::~SearchDeviceWidget()
{
    delete ui;
    delete addressesList;
}

void SearchDeviceWidget::handleSearchButtonClick()
{
    clearLayout(ui->verticalLayout_2);
    if (ui->checkBox->isChecked()) {
        emit broadcastSearch();
    } else {
        bool ok;
        startAddressSearch = ui->IntervalFrom->text().toInt(&ok);
        if (!ok) {
            qDebug()<<"from - not a number";
            return;
        }
        endAddressSearch = ui->IntervalTo->text().toInt(&ok);
        if (!ok) {
            qDebug()<<"to - not a number";
            return;
        }
        for (int i=startAddressSearch; i<=endAddressSearch; i++ ) {
            addressesList->append(i);
        }
        processCheckAddress();
    }
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

void SearchDeviceWidget::processCheckAddress()
{
    qDebug()<<"we are in processCheckAddress";
    int progress = (((endAddressSearch-startAddressSearch)-addressesList->size())/(endAddressSearch-startAddressSearch))*100;
    ui->SearchProgressBar->setValue(progress);
    if (!addressesList->size()) {
        return;
    }
    int currentAddress = addressesList->takeFirst();
    emit checkAddress(currentAddress);
}

void SearchDeviceWidget::handleCheckResult(bool hasDevice, int address)
{
    if (hasDevice) {
        ConnectDeviceButton *btn = new ConnectDeviceButton(address);
        ui->verticalLayout_2->addWidget(btn);
        connect(btn, &ConnectDeviceButton::connectDeviceClicked, this, [this](int address){
            qDebug()<<"connect device - " << address;
            emit connectDevice(address);
        });
    }

    qDebug()<<hasDevice<<"-"<<address;
    processCheckAddress();
    return;
}

void SearchDeviceWidget::handleCheckResult(bool hasDevice, int address, const QString& description)
{
    ConnectDeviceButton *btn = new ConnectDeviceButton(address, description);
    ui->verticalLayout_2->addWidget(btn);
    connect(btn, &ConnectDeviceButton::connectDeviceClicked, this, [this](int address){
        qDebug()<<"connect device - " << address;
        emit connectDevice(address);
    });
    qDebug()<<hasDevice<<"-"<<address<<"-"<<description;
    processCheckAddress();
    return;
}

void SearchDeviceWidget::handleCheckResult(const QMap<int, QString>& devices)
{
    for (auto it = devices.cbegin(); it != devices.cend(); ++it) {
        ConnectDeviceButton *btn = new ConnectDeviceButton(it.key(), it.value());
        ui->verticalLayout_2->addWidget(btn);
        connect(btn, &ConnectDeviceButton::connectDeviceClicked, this, [this](int address){
            qDebug()<<"connect device - " << address;
            emit connectDevice(address);
        });
        qDebug() << "key:" << it.key() << "value:" << it.value();
    }
    return;
}

void SearchDeviceWidget::clearLayout(QLayout* layout) {
    if (!layout)
        return;

    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) { // берём первый элемент
        if (QWidget* widget = item->widget()) {
            widget->setParent(nullptr);  // отвязываем виджет от layout
            delete widget;               // удаляем виджет
        }
        delete item;                    // удаляем QLayoutItem
    }
}


