/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2021 Nicholas Yoder
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef HOTKEY_H
#define HOTKEY_H

//#include "qxt/qxtglobalshortcut.h"
#include <QtDBus>
#include <QKeySequence>

#include <xcb/xcb.h>

enum SType
{
    Type_Exec,
    Type_Dbus
};

class globalhotkey : public QObject
{
    Q_OBJECT

public:
    globalhotkey(const QKeySequence &sequence, SType type);
    ~globalhotkey();

    //bool enabled;
    Qt::Key key;
    Qt::KeyboardModifiers mods;

    void setShortcut(const QKeySequence& shortcut);
    void unsetShortcut();

public slots:
    void setDbusInfo(QString service, QString path, QString interface, QString method, QString bus);
    void setExecCommand(const QString &command){shcommand=command;}

    void XcbEventFilter(xcb_generic_event_t *event);

private slots:
    void  exec();

private:
    SType shtype;
    QString shcommand;
    QString dbusservice;
    QString dbuspath;
    QString dbusinterface;
    QString dbusmethod;
    QString dbusbus;


    quint32 nativeKeycode(Qt::Key keycode);
    quint32 nativeModifiers(Qt::KeyboardModifiers modifiers);
    void registerShortcut(quint32 nativeKey, quint32 nativeMods);
    void unregisterShortcut(quint32 nativeKey, quint32 nativeMods);

};

#endif // HOTKEY_H
