QT       += core gui widgets dbus

TARGET = forest-settings
TEMPLATE = app

DESTDIR = ../usr/bin

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
    actionmanager.cpp \
        main.cpp \
        fsettings.cpp \
        listwidget.cpp \
    page.cpp \
    settingsmanager.cpp \
    widgets/keysequenceinput.cpp \
    widgets/lineedit.cpp \
    widgets/pluginlist.cpp

HEADERS += \
    actionmanager.h \
        fsettings.h \
        listwidget.h \
    page.h \
    settingsmanager.h \
    widgets/keysequenceinput.h \
    widgets/lineedit.h \
    widgets/pluginlist.h

# Default rules for deployment.
target.path = /usr/bin

INSTALLS += target
