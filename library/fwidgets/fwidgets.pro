QT += widgets

TARGET = fwidgets
TEMPLATE = lib

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    elidedlabel.cpp

HEADERS += \
    elidedlabel.h \
    fbasewidget.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
    INSTALLS += target
}
