QT += widgets dbus

TARGET = memorymonitor
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../../usr/lib/forest/panel

CONFIG += plugin

INCLUDEPATH = ../../panel-library

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
    memmon.cpp \
    settingswidget.cpp

HEADERS += \
    memmon.h \
    settingswidget.h \
    ../../panel-library/panelbutton.h \
    ../../panel-library/popup.h \
    ../../panel-library/popupmenu.h \
    ../../panel-library/graphwidget.h

# Default rules for deployment.

target.path = /usr/lib/forest/panel

INSTALLS += target

FORMS += \
    settingswidget.ui

LIBS += -L$$OUT_PWD/../../../library/miscutills/ -lmiscutills
INCLUDEPATH += $$PWD/../../../library/miscutills
DEPENDPATH += $$PWD/../../../library/miscutills
PRE_TARGETDEPS += $$OUT_PWD/../../../library/miscutills/libmiscutills.a
