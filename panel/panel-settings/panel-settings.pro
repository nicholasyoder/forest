QT += widgets dbus

TARGET = panel-settings
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../usr/lib/forest

CONFIG += c++17 plugin

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    panelsettings.cpp

HEADERS += \
    panelsettings.h

target.path = /usr/lib/forest
INSTALLS += target
