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
    hotkeys/edithotkeywidget.cpp \
    hotkeys/hotkeysettings.cpp \
    notifications/notificationssettings.cpp \
    servicessettings.cpp

HEADERS += \
    ../../library/pluginutills/settings_plugin_interface.h \
    hotkeys/edithotkeywidget.h \
    hotkeys/hotkeysettings.h \
    notifications/notificationssettings.h \
    servicessettings.h

FORMS += \
    hotkeys/edithotkeywidget.ui

target.path = /usr/lib/forest
INSTALLS += target
