QT += widgets dbus

TARGET = panel-settings
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../usr/lib/forest

CONFIG += c++17 plugin

include(../../shared-variables.pri)
include(../../library/miscutills/miscutills.pri)

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    panelsettings.cpp

HEADERS += \
    ../../library/pluginutills/settings_plugin_interface.h \
    ../../settings/widgets/listwidget.h \
    panelsettings.h

target.path = /usr/lib/forest
INSTALLS += target
