QT += widgets dbus

TARGET = sensors
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../../../usr/lib/forest/panel

CONFIG += plugin

INCLUDEPATH = ../../library

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
    ../../library/panelbutton.h \
    ../../library/popup.h \
    ../../library/popuprender.h \
    ../../library/popupmenu.h

FORMS  += widgetsensorconf.ui \


# Default rules for deployment.

target.path = /usr/lib/forest/panel

INSTALLS += target
