QT       += core gui widgets x11extras

TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += /usr/include/KF5/KWindowSystem

SOURCES += \
        numlock.cpp \
        xcbutills.cpp

HEADERS += \
        numlock.h \
        xcbutills.h

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    xcbutills.pri
