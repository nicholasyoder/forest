QT       += core gui dbus
CONFIG      +=  plugin debug_and_release
TARGET      = $$qtLibraryTarget(forestqxt)
DESTDIR = ../../usr/lib

TEMPLATE    = lib

LIBS        += -L. 


HEADERS  += \
    qxtglobalshortcut_p.h \
    qxtglobalshortcut.h \
    qxtglobal.h \
    keymapper_x11.h

SOURCES += \
    qxtglobalshortcut_x11.cpp \
    qxtglobalshortcut.cpp
#target.path = $$[QT_INSTALL_PLUGINS]/designer
target.path = /usr/lib
INSTALLS    += target
