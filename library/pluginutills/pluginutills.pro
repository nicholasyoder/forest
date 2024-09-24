QT += core gui widgets

TEMPLATE = lib
CONFIG += staticlib

HEADERS += \
    app_plugin_interface.h \
    pluginutills.h \
    settings_plugin_interface.h

SOURCES += \
    pluginutills.cpp


unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    pluginutills.pri

