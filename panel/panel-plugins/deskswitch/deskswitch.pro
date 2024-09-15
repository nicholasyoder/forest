QT       += core gui dbus widgets x11extras

TARGET = deskswitch
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../../usr/lib/forest/panel

CONFIG += plugin

INCLUDEPATH += ../../panel-library
INCLUDEPATH += ../../../library

LIBS += -L/usr/lib/x86_64-linux-gnu/ -lKF5WindowSystem
INCLUDEPATH += /usr/include/KF5/KWindowSystem
DEPENDPATH += /usr/include/KF5/KWindowSystem

include(../../../shared-variables.pri)
include(../../../library/miscutills/miscutills.pri)
include(../../../library/xcbutills/xcbutills.pri)

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

SOURCES += \
    deskswitch.cpp \
    deskbutton.cpp

HEADERS += \
    deskswitch.h \
    ../../panel-library/panelbutton.h \
    ../../panel-library/popup.h \
    ../../panel-library/popupmenu.h \
    deskbutton.h

# Default rules for deployment.
target.path = /usr/lib/forest/panel

INSTALLS += target
