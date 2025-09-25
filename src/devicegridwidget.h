#pragma once

#include <QWidget>
#include <QVector>

struct Device {
    bool available = false;
};

class DeviceGridWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceGridWidget(int initialCount = 247, QWidget *parent = nullptr);

    // Настройки внешнего вида
    void setSquareSize(int size) { m_squareSize = size; update(); }
    void setSpacing(int spacing) { m_spacing = spacing; update(); }

    // Установить количество устройств
    void setDeviceCount(int count);

public slots:
    // Слот для обновления статуса устройства
    void setDeviceStatus(int address, bool available);
    void addDevice(bool available = false);

signals:
    void deviceClicked(int address);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QRect squareRectForIndex(int index) const;
    int columns() const;

private:
    QVector<Device> m_devices;
    int m_squareSize = 10;
    int m_spacing = 5;
    int m_lastCols = 0;
};
