QT += widgets dbus x11extras

TARGET = system-settings
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../usr/lib/forest

CONFIG += c++17 plugin x11

LIBS += -lX11 -lXcursor

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    aboutpage.cpp \
    cursorthemesettings.cpp \
    forestthemesettings.cpp \
    systemsettings.cpp

HEADERS += \
    ../../library/pluginutills/settings_plugin_interface.h \
    ../../settings/widgets/listwidget.h \
    aboutpage.h \
    cursorthemesettings.h \
    forestthemesettings.h \
    systemsettings.h

target.path = /usr/lib/forest
INSTALLS += target


LIBS += -L$$OUT_PWD/../../library/miscutills/ -lmiscutills
INCLUDEPATH += $$PWD/../../library/miscutills
DEPENDPATH += $$PWD/../../library/miscutills
PRE_TARGETDEPS += $$OUT_PWD/../../library/miscutills/libmiscutills.a
