QT       += core gui widgets

TARGET = forest-settings
TEMPLATE = app

DESTDIR = ../usr/bin

CONFIG += c++17

include(../shared-variables.pri)
include(../library/pluginutills/pluginutills.pri)

SOURCES += \
    breadcrumbwidget.cpp \
    catlistwidget.cpp \
    main.cpp \
    settingsmanager.cpp

HEADERS += \
    ../library/pluginutills/settings_plugin_interface.h \
    breadcrumbwidget.h \
    catlistwidget.h \
    settingsmanager.h \
    widgets/listwidget.h

# Default rules for deployment.
target.path = /usr/bin
INSTALLS += target
