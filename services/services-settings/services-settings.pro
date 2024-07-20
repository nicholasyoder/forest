QT += widgets dbus

TARGET = services-settings
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../usr/lib/forest

CONFIG += c++17 plugin

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    hotkeysettings.cpp \
    servicessettings.cpp

HEADERS += \
    ../../library/pluginutills/settings_plugin_interface.h \
    hotkeysettings.h \
    servicessettings.h

target.path = /usr/lib/forest
INSTALLS += target
