QT       += core gui dbus widgets

TARGET = nmcontrol
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../../usr/lib/forest/panel

CONFIG += plugin
CONFIG += c++11

INCLUDEPATH = ../../panel-library

include(../../../shared-variables.pri)
include(../../../library/miscutills/miscutills.pri)

SOURCES += \
    nmcontrol.cpp

HEADERS += \
    nmcontrol.h \
    ../../panel-library/panelbutton.h \
    ../../panel-library/popup.h \
    ../../panel-library/popupmenu.h

# Default rules for deployment.
target.path = /usr/lib/forest/panel

INSTALLS += target
