QT += widgets dbus

TARGET = volume
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../../usr/lib/forest/panel

CONFIG += plugin

INCLUDEPATH = ../../panel-library

LIBS += -lasound -lQt5Xdg

include(../../../shared-variables.pri)
include(../../../library/miscutills/miscutills.pri)

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    alsadevice.cpp \
    alsaengine.cpp \
    audiodevice.cpp \
    audioengine.cpp \
    fvolume.cpp

HEADERS += \
    ../../panel-library/panelbutton.h \
    ../../panel-library/popup.h \
    ../../panel-library/popupmenu.h \
    alsadevice.h \
    alsaengine.h \
    audiodevice.h \
    audioengine.h \
    fvolume.h

# Default rules for deployment.

target.path = /usr/lib/forest/panel

INSTALLS += target
