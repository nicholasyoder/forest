QT       += core gui dbus widgets

TARGET = nmcontrol
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../../usr/lib/forest/panel

CONFIG += plugin
CONFIG += c++11

INCLUDEPATH = ../../library

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    nmcontrol.cpp

HEADERS += \
    nmcontrol.h \
    ../../library/panelbutton.h \
    ../../library/popup.h \
    ../../library/popuprender.h \
    ../../library/popupmenu.h

# Default rules for deployment.
target.path = /usr/lib/forest/panel

INSTALLS += target
