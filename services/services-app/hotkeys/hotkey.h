// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HOTKEY_H
#define HOTKEY_H

//#include "qxt/qxtglobalshortcut.h"
#include <QtDBus>
#include <QKeySequence>

#include <xcb/xcb.h>

enum HK_Type
{
    Type_Exec,
    Type_Dbus
};

class globalhotkey : public QObject
{
    Q_OBJECT

public:
    globalhotkey(const QKeySequence &sequence, HK_Type type);
    ~globalhotkey();

    void setShortcut(const QKeySequence& shortcut);
    void unsetShortcut();

public slots:
    void setDbusInfo(QString service, QString path, QString interface, QString method, QString bus);
    void setExecCommand(const QString &command){shcommand=command;}

    void XcbEventFilter(xcb_generic_event_t *event);

private slots:
    void  exec();

private:
    HK_Type hotkey_type;
    QString shcommand;
    QString dbusservice;
    QString dbuspath;
    QString dbusinterface;
    QString dbusmethod;
    QString dbusbus;

    xcb_keycode_t keycode;
    quint32 modmask;

    xcb_keycode_t lastkeypressed = 0;

    quint32 nativeKeycode(Qt::Key keycode);
    quint32 nativeModifiers(Qt::KeyboardModifiers modifiers);
    void registerShortcut(quint32 nativeKey, quint32 nativeMods);
    void unregisterShortcut(quint32 nativeKey, quint32 nativeMods);

};

#endif // HOTKEY_H
