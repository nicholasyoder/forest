QT += widgets dbus x11extras

TARGET = forest-services
TEMPLATE = lib
DEFINES += PLUG_LIBRARY

DESTDIR = ../usr/lib/forest

CONFIG += c++11 \
            plugin

LIBS += -lpolkit-qt5-agent-1 -lQt5Xdg
# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    hotkeys/foresthotkeys.cpp \
    hotkeys/hotkey.cpp \
    hotkeys/editshortcutwidget.cpp \
    services.cpp \
    notifications/notify.cpp \
    notifications/notifyadapter.cpp \
    notifications/notifypopup.cpp \
    polkit/polkitagent.cpp \ 
    polkit/polkitdialog.cpp \
    ../library/xcbutills/xcbutills.cpp

HEADERS += \
    hotkeys/foresthotkeys.h \
    hotkeys/hotkey.h \
    hotkeys/qxt/keymapper_x11.h \
    hotkeys/editshortcutwidget.h \
    services.h \
    notifications/notify.h \
    notifications/notifyadapter.h \
    notifications/notifypopup.h \
    polkit/polkitagent.h \ 
    polkit/polkitdialog.h \
    ../library/xcbutills/xcbutills.h

FORMS += \
    hotkeys/editshortcutwidget.ui \
    polkit/polkitdialog.ui

# Default rules for deployment.
target.path = /usr/lib/forest

INSTALLS += target

#unix:!macx: LIBS += -L$$OUT_PWD/../library/xcbutills/ -lxcbutills

#INCLUDEPATH += $$PWD/../library/xcbutills
#DEPENDPATH += $$PWD/../library/xcbutills

#unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/../library/xcbutills/libxcbutills.a
