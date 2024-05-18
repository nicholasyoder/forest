QT += widgets dbus

TARGET = desktop-settings
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../usr/lib/forest

CONFIG += c++17 plugin

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    desktopsettings.cpp

HEADERS += \
    ../../library/pluginutills/settings_plugin_interface.h \
    desktopsettings.h

target.path = /usr/lib/forest
INSTALLS += target

FORMS +=
