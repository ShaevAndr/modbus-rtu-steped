#pragma once
#include <QPushButton>
#include <QString>

class ConnectDeviceButton : public QPushButton {
    Q_OBJECT

public:
    explicit ConnectDeviceButton(int address,
                                  QString description = QString(),
                                  QWidget* parent = nullptr);

    int address() const { return m_address; }

signals:
    void connectDeviceClicked(int address);

private:
    int m_address;
    QString m_description;

    static QString formatText(int address, const QString& description = QString());
};
