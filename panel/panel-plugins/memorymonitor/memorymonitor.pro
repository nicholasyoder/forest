QT += widgets dbus

TARGET = memorymonitor
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../../usr/lib/forest/panel

CONFIG += plugin

INCLUDEPATH = ../../panel-library

include(../../../shared-variables.pri)
include(../../../library/miscutills/miscutills.pri)

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    memmon.cpp \
    settingswidget.cpp

HEADERS += \
    memmon.h \
    settingswidget.h \
    ../../panel-library/panelbutton.h \
    ../../panel-library/popup.h \
    ../../panel-library/popupmenu.h \
    ../../panel-library/graphwidget.h

# Default rules for deployment.

target.path = /usr/lib/forest/panel

INSTALLS += target

FORMS += \
    settingswidget.ui
