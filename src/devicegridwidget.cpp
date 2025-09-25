#include "DeviceGridWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QDebug>

DeviceGridWidget::DeviceGridWidget(int initialCount, QWidget *parent)
    : QWidget(parent)
{
    m_devices.resize(initialCount);
    setMinimumSize(100, 100);
    m_lastCols = 0;
}

void DeviceGridWidget::setDeviceCount(int count)
{
    if (count < 0) return;
    m_devices.resize(count);
    update();
}

void DeviceGridWidget::addDevice(bool available)
{
    Device d;
    d.available = available;
    m_devices.append(d);
    update();
}

void DeviceGridWidget::setDeviceStatus(int address, bool available)
{
    qDebug()<<"[DeviceGridWidget] set device status" << address << available;
    if (address < 0) return;

    if (address >= m_devices.size()) {
        m_devices.resize(address + 1);
    }

    int curCols = columns();
    if (curCols != m_lastCols) {
        m_devices[address].available = available;
        m_lastCols = curCols;
        update();
        return;
    }

    m_devices[address].available = available;
    QRect r = squareRectForIndex(address).adjusted(-1, -1, 1, 1);
    update(r);
}

void DeviceGridWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int cols = columns();
    m_lastCols = cols;
    const int per = m_squareSize + m_spacing;

    for (int i = 0; i < m_devices.size(); ++i) {
        QRect rect = squareRectForIndex(i);

        QColor fill = m_devices[i].available ? QColor(46, 204, 113) : QColor(231, 76, 60);
        p.fillRect(rect, fill);
        p.setPen(Qt::black);
        p.drawRect(rect);
    }
}

void DeviceGridWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    QPoint pos = event->pos();

    int clickedIndex = -1;
    for (int i = 0; i < m_devices.size(); ++i) {
        if (squareRectForIndex(i).contains(pos)) {
            clickedIndex = i;
            break;
        }
    }

    if (clickedIndex >= 0) {
        qDebug() << "Device clicked:" << clickedIndex;
        emit deviceClicked(clickedIndex);
    } else {
        addDevice(false);
    }
}

void DeviceGridWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    int cols = columns();
    if (cols != m_lastCols) {
        m_lastCols = cols;
        update();
    }
}

QRect DeviceGridWidget::squareRectForIndex(int index) const
{
    int cols = columns();
    if (cols <= 0) cols = 1;
    const int per = m_squareSize + m_spacing;
    int row = index / cols;
    int col = index % cols;
    int x = m_spacing + col * per;
    int y = m_spacing + row * per;
    return QRect(x, y, m_squareSize, m_squareSize);
}

int DeviceGridWidget::columns() const
{
    const int per = m_squareSize + m_spacing;
    if (per <= 0) return 1;
    int avail = width() - m_spacing;
    if (avail <= 0) return 1;
    int cols = avail / per;
    return qMax(1, cols);
}
