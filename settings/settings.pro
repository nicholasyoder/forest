QT       += core gui widgets

TARGET = forest-settings
TEMPLATE = app

DESTDIR = ../usr/bin

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    breadcrumbwidget.cpp \
    catlistwidget.cpp \
    main.cpp \
    settingsmanager.cpp

HEADERS += \
    ../library/pluginutills/settings_plugin_interface.h \
    breadcrumbwidget.h \
    catlistwidget.h \
    settingsmanager.h \
    widgets/listwidget.h

# Default rules for deployment.
target.path = /usr/bin
INSTALLS += target

# pluginutills
LIBS += -L$$OUT_PWD/../library/pluginutills/ -lpluginutills
INCLUDEPATH += $$PWD/../library/pluginutills
DEPENDPATH += $$PWD/../library/pluginutills
PRE_TARGETDEPS += $$OUT_PWD/../library/pluginutills/libpluginutills.a
