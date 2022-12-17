QT       += core gui dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mainmenu
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../../usr/lib/forest/panel

CONFIG += plugin

INCLUDEPATH = ../../library

LIBS += -lQt5Xdg
# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

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
    ../../library/popup.h \
    ../../library/popuprender.h \
    ../../library/popupmenu.h \
    ../../library/panelbutton.h \
    menuitem.h \
    menupage.h \
    propertieswidget.h

FORMS += \
    propertieswidget.ui

# Default rules for deployment.
target.path = /usr/lib/forest/panel

INSTALLS += target
