QT += widgets dbus

TARGET = batterymonitor
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../../usr/lib/forest/panel

CONFIG += c++11 \
            plugin

INCLUDEPATH = ../../panel-library

include(../../../shared-variables.pri)
include(../../../library/miscutills/miscutills.pri)

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    battery.cpp \
    batterymonitor.cpp

HEADERS += \
    battery.h \
    batterymonitor.h \
    ../../panel-library/panelbutton.h \
    ../../panel-library/popup.h \
    ../../panel-library/popupmenu.h \
    ../../panel-library/buttonrender.h

# Default rules for deployment.
target.path = /usr/lib/forest/panel

INSTALLS += target
