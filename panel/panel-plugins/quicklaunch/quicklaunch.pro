QT += widgets dbus

TARGET = quicklaunch
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../../usr/lib/forest/panel

CONFIG += plugin

INCLUDEPATH = ../../panel-library

LIBS += -lQt5Xdg

include(../../../shared-variables.pri)
include(../../../library/miscutills/miscutills.pri)

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    launcher.cpp \
    quicklaunch.cpp

HEADERS += \
    launcher.h \
    quicklaunch.h \
    ../../panel-library/panelbutton.h \
    ../../panel-library/popup.h \
    ../../panel-library/popupmenu.h

# Default rules for deployment.
target.path = /usr/lib/forest/panel

INSTALLS += target

FORMS +=
