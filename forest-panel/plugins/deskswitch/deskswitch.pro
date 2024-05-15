QT       += core gui dbus widgets x11extras

TARGET = deskswitch
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../../usr/lib/forest/panel

CONFIG += plugin

INCLUDEPATH += ../../library
INCLUDEPATH += ../../../library

LIBS += -L/usr/lib/x86_64-linux-gnu/ -lKF5WindowSystem
INCLUDEPATH += /usr/include/KF5/KWindowSystem
DEPENDPATH += /usr/include/KF5/KWindowSystem

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
    deskswitch.cpp \
    deskbutton.cpp

HEADERS += \
    deskswitch.h \
    ../../library/panelbutton.h \
    ../../library/popup.h \
    ../../library/popuprender.h \
    ../../library/popupmenu.h \
    deskbutton.h

# Default rules for deployment.
target.path = /usr/lib/forest/panel

INSTALLS += target

# xcbutils
LIBS += -L$$OUT_PWD/../../../library/xcbutills/ -lxcbutills
INCLUDEPATH += $$PWD/../../../library/xcbutills
DEPENDPATH += $$PWD/../../../library/xcbutills
PRE_TARGETDEPS += $$OUT_PWD/../../../library/xcbutills/libxcbutills.a
