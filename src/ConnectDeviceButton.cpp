#include <QPushButton>
#include "ConnectDeviceButton.h"


ConnectDeviceButton::ConnectDeviceButton (int address, QString description, QWidget* parent) :
    QPushButton(formatText(address, description), parent),
    m_address(address),
    m_description(description) {
        connect(this, &QPushButton::clicked, this, [this](){
        emit connectDeviceClicked(m_address);
   });
}

QString ConnectDeviceButton::formatText(int address, const QString& description){
    QString addressString = QString::number(address);
    if (description.isEmpty()) {
        return addressString + "( нет описания )";
    }
    return addressString + "( " + description + " )";
}
