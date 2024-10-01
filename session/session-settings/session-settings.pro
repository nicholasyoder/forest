QT += widgets

TARGET = session-settings
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../usr/lib/forest

CONFIG += c++17 plugin x11

include(../../shared-variables.pri)
include(../../library/miscutills/miscutills.pri)

DEFINES += QT_DEPRECATED_WARNINGS

target.path = /usr/lib/forest
INSTALLS += target

HEADERS += \
    sessionsettings.h

SOURCES += \
    sessionsettings.cpp
