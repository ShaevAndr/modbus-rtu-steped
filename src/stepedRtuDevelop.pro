QT += core gui widgets serialport

CONFIG += c++17

SOURCES += \
    ConnectDeviceButton.cpp \
    main.cpp \
    mainwindow.cpp \
    searchdevicewidget.cpp \
    serialporttransport.cpp \
    modbuscrc.cpp \
    modbusrtuprotocol.cpp \
    settings.cpp \
    master.cpp \
    devicescandialog.cpp\

HEADERS += \
    ConnectDeviceButton.h \
    IProtocol.h \
    command.h \
    mainwindow.h \
    searchdevicewidget.h \
    transport.h \
    serialporttransport.h \
    master.h \
    modbuscrc.h \
    modbusrtuprotocol.h \
    errorcodes.h \
    settings.h \
    request.h \
    devicescandialog.h \
    device.h\


FORMS += \
    mainwindow.ui \
    searchdevicewidget.ui
