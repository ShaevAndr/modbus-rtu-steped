QT += core gui widgets serialport

CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    serialporttransport.cpp \
    modbuscrc.cpp \
    modbusrtuprotocol.cpp \
    settings.cpp \
    master.cpp

HEADERS += \
    IProtocol.h \
    command.h \
    mainwindow.h \
    transport.h \
    serialporttransport.h \
    master.h \
    modbuscrc.h \
    modbusrtuprotocol.h \
    errorcodes.h \
    settings.h \
