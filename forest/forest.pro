QT       += core gui dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = forest
TEMPLATE = app

DESTDIR = ../usr/bin

include(../shared-variables.pri)
include(../library/pluginutills/pluginutills.pri)
include(../library/miscutills/miscutills.pri)

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

SOURCES += \
        main.cpp \
        forest.cpp \
        fadewidget.cpp

HEADERS += \
        forest.h \
    forestxcbeventfilter.h \
        fadewidget.h

# Default rules for deployment.
target.path = /usr/bin

INSTALLS += target
