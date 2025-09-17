QT += core gui widgets serialport

CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    irxtx.cpp \
    modbuscommand.cpp \
    modbuscrc.cpp \
    modbusrtuprotocol.cpp \
    settings.cpp \

HEADERS += \
    IProtocol.h \
    command.h \
    mainwindow.h \
    irxtx.h \
    master.h \
    modbuscommand.h \
    modbuscrc.h \
    modbusrtuprotocol.h \
    settings.h \
