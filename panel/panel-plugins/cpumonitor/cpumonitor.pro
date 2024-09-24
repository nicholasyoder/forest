QT += widgets dbus

TARGET = cpumonitor
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../../usr/lib/forest/panel

CONFIG += plugin

INCLUDEPATH = ../../panel-library

include(../../../shared-variables.pri)
include(../../../library/miscutills/miscutills.pri)

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    cpumon.cpp \
    settingswidget.cpp

HEADERS += \
    cpumon.h \
    settingswidget.h \
    ../../panel-library/panelbutton.h \
    ../../panel-library/popup.h \
    ../../panel-library/popupmenu.h \
    ../../panel-library/graphwidget.h

FORMS += \
    settingswidget.ui

# Default rules for deployment.

target.path = /usr/lib/forest/panel

INSTALLS += target
