QT += widgets dbus

TARGET = desktop-app
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../usr/lib/forest

CONFIG += c++11 \
            plugin

LIBS += -lQt5Xdg

include(../../shared-variables.pri)
include(../../library/miscutills/miscutills.pri)

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    desktop.cpp \
    global_settings.cpp \
    wallpaperwidget.cpp \
    iconswidget.cpp \
    desktopicon.cpp

HEADERS += \
    desktop.h \
    global_settings.h \
    wallpaperwidget.h \
    iconswidget.h \
    desktopicon.h

# Default rules for deployment.
target.path = /usr/lib/forest

INSTALLS += target
