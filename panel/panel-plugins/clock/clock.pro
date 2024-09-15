QT       += core gui dbus widgets

TARGET = clock
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../../usr/lib/forest/panel

CONFIG += plugin

INCLUDEPATH = ../../panel-library

include(../../../shared-variables.pri)
include(../../../library/miscutills/miscutills.pri)

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

SOURCES += \
    clock.cpp \
    clocksettingswidget.cpp

HEADERS += \
    clock.h \
    ../../panel-library/panelbutton.h \
    ../../panel-library/popup.h \
    ../../panel-library/popupmenu.h \
    clocksettingswidget.h

# Default rules for deployment.
target.path = /usr/lib/forest/panel

INSTALLS += target

FORMS += \
    clocksettingswidget.ui
