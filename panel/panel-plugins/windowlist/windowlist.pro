QT += widgets dbus x11extras

TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../../usr/lib/forest/panel
INCLUDEPATH += ../../panel-library
INCLUDEPATH += ../../../library

LIBS += -L/usr/lib/x86_64-linux-gnu/ -lKF5WindowSystem
INCLUDEPATH += /usr/include/KF5/KWindowSystem
DEPENDPATH += /usr/include/KF5/KWindowSystem

CONFIG += c++11 \
            plugin

include(../../../shared-variables.pri)
include(../../../library/miscutills/miscutills.pri)
include(../../../library/xcbutills/xcbutills.pri)

# Must be after xcbutils include
LIBS += -lxcb-image

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    imagepopup.cpp \
    settingswidget.cpp \
    windowbutton.cpp \
    windowlist.cpp

HEADERS += \
    closebutton.h \
    imagepopup.h \
    settingswidget.h \
    windowbutton.h \
    windowlist.h \
    ../../panel-library/panelbutton.h \
    ../../panel-library/popup.h \
    ../../panel-library/popupmenu.h

# Default rules for deployment.

target.path = /usr/lib/forest/panel

INSTALLS += target

FORMS += \
    settingswidget.ui
