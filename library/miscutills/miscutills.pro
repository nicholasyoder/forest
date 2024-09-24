QT += core dbus

TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    miscutills.cpp

HEADERS += \
    miscutills.h


unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    miscutills.pri

