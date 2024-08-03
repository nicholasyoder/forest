QT       += core gui widgets

TARGET = forest-session
TEMPLATE = app

DESTDIR = ../../usr/bin

CONFIG += c++17

LIBS += -lQt5Xdg

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    sessionapp.cpp

HEADERS += \
    sessionapp.h


target.path = /usr/bin
INSTALLS += target
