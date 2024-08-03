QT += widgets dbus

TARGET = sensors
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../../usr/lib/forest/panel

CONFIG += plugin

INCLUDEPATH = ../../panel-library

LIBS += -lsensors

SOURCES +=\
    widgetsensorconf.cpp \
    sensorwidget.cpp \
    sensor/sensors.cpp \
    sensor/chip.cpp \
    sensor/feature.cpp

HEADERS  +=\
    widgetsensorconf.h \
    sensorwidget.h\
    sensor/sensors.h \
    sensor/chip.h \
    sensor/feature.h \
    ../../panel-library/panelbutton.h \
    ../../panel-library/popup.h \
    ../../panel-library/popupmenu.h

FORMS  += widgetsensorconf.ui \


# Default rules for deployment.

target.path = /usr/lib/forest/panel

INSTALLS += target

LIBS += -L$$OUT_PWD/../../../library/miscutills/ -lmiscutills
INCLUDEPATH += $$PWD/../../../library/miscutills
DEPENDPATH += $$PWD/../../../library/miscutills
PRE_TARGETDEPS += $$OUT_PWD/../../../library/miscutills/libmiscutills.a
