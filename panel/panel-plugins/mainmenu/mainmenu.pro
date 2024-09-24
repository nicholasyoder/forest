QT       += core gui dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mainmenu
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../../usr/lib/forest/panel

CONFIG += plugin

INCLUDEPATH += ../../panel-library
INCLUDEPATH += ../../../library

LIBS += -lQt5Xdg

include(../../../shared-variables.pri)
include(../../../library/miscutills/miscutills.pri)

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

SOURCES += \
    apploader.cpp \
    contextmenu.cpp \
    itemhandler.cpp \
        mainmenu.cpp \
    menuitem.cpp \
    menupage.cpp \
    propertieswidget.cpp

HEADERS += \
    apploader.h \
    contextmenu.h \
    globals.h \
    itemhandler.h \
        mainmenu.h \
    ../../panel-library/popup.h \
    ../../panel-library/popupmenu.h \
    ../../panel-library/panelbutton.h \
    menuitem.h \
    menupage.h \
    propertieswidget.h

FORMS += \
    propertieswidget.ui

# Default rules for deployment.
target.path = /usr/lib/forest/panel

INSTALLS += target
