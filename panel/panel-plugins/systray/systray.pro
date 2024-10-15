QT       += core gui dbus widgets x11extras

TARGET = systray
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../../usr/lib/forest/panel

CONFIG += plugin

LIBS += $(SUBLIBS) -lXrender -lxcb-composite -lXdamage -lXfixes

INCLUDEPATH = ../../panel-library
INCLUDEPATH += ../../../library

LIBS += -L/usr/lib/x86_64-linux-gnu/ -lKF5WindowSystem
INCLUDEPATH += /usr/include/KF5/KWindowSystem
DEPENDPATH += /usr/include/KF5/KWindowSystem

include(../../../shared-variables.pri)
include(../../../library/miscutills/miscutills.pri)
include(../../../library/xcbutills/xcbutills.pri)

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

SOURCES += \
        systray.cpp \
    fdoselectionmanager.cpp \
    snidbus.cpp \
    sniproxy.cpp \
    xtestsender.cpp \

HEADERS += \
        systray.h \
    trayicon.h \
    snidbus.h \
    sniproxy.h \
    xtestsender.h \
    xcbutils.h \
    fdoselectionmanager.h \
    ../../panel-library/panelbutton.h \
    ../../panel-library/buttonrender.h

# Default rules for deployment.
target.path = /usr/lib/forest/panel

INSTALLS += target
