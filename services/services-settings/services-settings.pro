QT += widgets dbus

TARGET = services-settings
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../usr/lib/forest

CONFIG += c++17 plugin

include(../../shared-variables.pri)
include(../../library/miscutills/miscutills.pri)

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    edithotkeywidget.cpp \
    hotkeysettings.cpp \
    servicessettings.cpp

HEADERS += \
    ../../library/pluginutills/settings_plugin_interface.h \
    edithotkeywidget.h \
    hotkeysettings.h \
    servicessettings.h

FORMS += \
    edithotkeywidget.ui

target.path = /usr/lib/forest
INSTALLS += target
