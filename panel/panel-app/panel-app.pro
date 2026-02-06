QT       += core gui dbus widgets

TARGET = panel-app
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../usr/lib/forest

CONFIG += plugin

LIBS += -L/usr/lib/x86_64-linux-gnu/ -lKF6WindowSystem
INCLUDEPATH += /usr/include/KF6/KWindowSystem
DEPENDPATH += /usr/include/KF6/KWindowSystem

include(../../shared-variables.pri)
include(../../library/xcbutills/xcbutills.pri)
include(../../library/miscutills/miscutills.pri)

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

SOURCES += \
    autohidemanager.cpp \
    geometrymanager.cpp \
    hiddenpanel.cpp \
    panel.cpp

HEADERS += \
    autohidemanager.h \
    geometrymanager.h \
    hiddenpanel.h \
    panel.h \
    ../panel-library/panelpluginterface.h \
    ../panel-library/panelbutton.h \
    ../panel-library/popupmenu.h \
    ../panel-library/popup.h \

FORMS +=

# Default rules for deployment.
target.path = /usr/lib/forest

INSTALLS += target
