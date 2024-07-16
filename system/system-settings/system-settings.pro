QT += widgets dbus

TARGET = system-settings
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../usr/lib/forest

CONFIG += c++17 plugin

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    systemsettings.cpp

HEADERS += \
    ../../library/pluginutills/settings_plugin_interface.h \
    ../../settings/widgets/listwidget.h \
    systemsettings.h

target.path = /usr/lib/forest
INSTALLS += target


LIBS += -L$$OUT_PWD/../../library/miscutills/ -lmiscutills
INCLUDEPATH += $$PWD/../../library/miscutills
DEPENDPATH += $$PWD/../../library/miscutills
PRE_TARGETDEPS += $$OUT_PWD/../../library/miscutills/libmiscutills.a
