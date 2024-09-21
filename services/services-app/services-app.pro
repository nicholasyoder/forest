QT += widgets dbus x11extras

TARGET = services-app
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../usr/lib/forest

CONFIG += c++11 \
            plugin

LIBS += -lpolkit-qt5-agent-1 -lQt5Xdg

LIBS += -L/usr/lib/x86_64-linux-gnu/ -lKF5WindowSystem
INCLUDEPATH += /usr/include/KF5/KWindowSystem
DEPENDPATH += /usr/include/KF5/KWindowSystem

include(../../shared-variables.pri)
include(../../library/xcbutills/xcbutills.pri)

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    hotkeys/foresthotkeys.cpp \
    hotkeys/hotkey.cpp \
    services.cpp \
    notifications/notify.cpp \
    notifications/notifyadapter.cpp \
    notifications/notifypopup.cpp \
    polkit/polkitagent.cpp \ 
    polkit/polkitdialog.cpp

HEADERS += \
    hotkeys/foresthotkeys.h \
    hotkeys/hotkey.h \
    hotkeys/qxt/keymapper_x11.h \
    services.h \
    notifications/notify.h \
    notifications/notifyadapter.h \
    notifications/notifypopup.h \
    polkit/polkitagent.h \ 
    polkit/polkitdialog.h

FORMS += \
    polkit/polkitdialog.ui

# Default rules for deployment.
target.path = /usr/lib/forest

INSTALLS += target
