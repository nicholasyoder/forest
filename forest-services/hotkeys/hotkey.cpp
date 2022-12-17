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

#include "hotkey.h"

#include <QX11Info>
#include <X11/Xlib.h>
#include "qxt/keymapper_x11.h"

globalhotkey::globalhotkey(const QKeySequence& sequence,SType type) : shtype(type)
{
    setShortcut(sequence);
    qDebug() << "Add Shortcut"<< sequence.toString();
    //connect(this,SIGNAL(activated()),this,SLOT(exec()));
}

globalhotkey::~globalhotkey()
{
    unsetShortcut();
}

void globalhotkey::setDbusInfo(QString service, QString path,QString interface, QString method, QString bus)
{
    dbusservice = service;
    dbuspath = path;
    dbusinterface = interface;
    dbusmethod = method;
    dbusbus = bus;
}

void globalhotkey::exec()
{
    if(shtype==Type_Exec)
    {
      QProcess::startDetached(shcommand);
    }
    else
    {
        if (dbusbus == "Session" || dbusbus == "")
        {
            QDBusInterface dbus(dbusservice, dbuspath, dbusinterface, QDBusConnection::sessionBus());
            if (!dbus.isValid())
            {
                qDebug() << "QDBusInterface is not valid!";
                return ;
            }
            dbus.call(dbusmethod);
        }
        else
        {
            QDBusInterface dbus(dbusservice, dbuspath, dbusinterface, QDBusConnection::systemBus());
            if (!dbus.isValid())
            {
                qDebug() << "QDBusInterface is not valid!";
                return ;
            }
            dbus.call(dbusmethod);
        }

    }
}

void globalhotkey::XcbEventFilter(xcb_generic_event_t *event)
{
    if (event->response_type == XCB_KEY_PRESS)
    {
        xcb_key_press_event_t *keyevent = (xcb_key_press_event_t*)event;

        //qDebug() << "key: " << keyevent->detail;

        quint32 nativeKey = keyevent->detail;
        quint32 nativeMods = keyevent->state & (ShiftMask | ControlMask | Mod1Mask | Mod4Mask);

        if (nativeKey == nativeKeycode(key) && nativeMods == nativeModifiers(mods))
            exec();
    }
    //else if (event->response_type == XCB_KEY_RELEASE)
    //{
    //    qDebug() << "release";
    //    xcb_key_release_event_t *keyevent = (xcb_key_release_event_t*)event;
    //    quint32 nativeKey = keyevent->detail;
    //}
}

void globalhotkey::setShortcut(const QKeySequence& shortcut)
{
          Qt::KeyboardModifiers allMods = Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier;
          key = shortcut.isEmpty() ? Qt::Key(0) : Qt::Key((shortcut[0] ^ allMods) & shortcut[0]);
          mods = shortcut.isEmpty() ? Qt::KeyboardModifiers(0) : Qt::KeyboardModifiers(shortcut[0] & allMods);
          const quint32 nativeKey = nativeKeycode(key);
          const quint32 nativeMods = nativeModifiers(mods);
          registerShortcut(nativeKey, nativeMods);
}

void globalhotkey::unsetShortcut()
{
          const quint32 nativeKey = nativeKeycode(key);
          const quint32 nativeMods = nativeModifiers(mods);
          unregisterShortcut(nativeKey, nativeMods);
          key = Qt::Key(0);
          mods = Qt::KeyboardModifiers(0);
}

quint32 globalhotkey::nativeModifiers(Qt::KeyboardModifiers modifiers)
{
          // ShiftMask, LockMask, ControlMask, Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask, and Mod5Mask
          quint32 native = 0;
          if (modifiers & Qt::ShiftModifier)
                    native |= ShiftMask;
          if (modifiers & Qt::ControlModifier)
                    native |= ControlMask;
          if (modifiers & Qt::AltModifier)
                    native |= Mod1Mask;
          if (modifiers & Qt::MetaModifier)
                    native |= Mod4Mask;
          // TODO: resolve these?
          //if (modifiers & Qt::KeypadModifier)
          //if (modifiers & Qt::GroupSwitchModifier)
          return native;
}

quint32 globalhotkey::nativeKeycode(Qt::Key key)
{
          // (davidsansome) Try the table from QKeyMapper first - this seems to be
          // the only way to get Keysyms for the media keys.
          unsigned int keysym = 0;
          int i = 0;
          while (KeyTbl[i]) {
                    if (KeyTbl[i+1] == static_cast<uint>(key)) {
                              keysym = KeyTbl[i];
                              break;
                    }
                    i += 2;
          }

          // If that didn't work then fall back on XStringToKeysym
          if (!keysym) {
                    keysym = XStringToKeysym(QKeySequence(key).toString().toLatin1().data());
          }

          Display* display = QX11Info::display();
          return XKeysymToKeycode(display, keysym);
}

void globalhotkey::registerShortcut(quint32 nativeKey, quint32 nativeMods)
{
          Display* display = QX11Info::display();
          Window window = QX11Info::appRootWindow();
          Bool owner = True;
          int pointer = GrabModeAsync;
          int keyboard = GrabModeAsync;
          //error = false;
          //original_x_errhandler = XSetErrorHandler(qxt_x_errhandler);
          XGrabKey(display, nativeKey, nativeMods, window, owner, pointer, keyboard);
          XGrabKey(display, nativeKey, nativeMods | Mod2Mask, window, owner, pointer, keyboard); // allow numlock
          XSync(display, False);
          //XSetErrorHandler(original_x_errhandler);
          //return !error;
}

void globalhotkey::unregisterShortcut(quint32 nativeKey, quint32 nativeMods)
{
          Display* display = QX11Info::display();
          Window window = QX11Info::appRootWindow();
          //error = false;
          //original_x_errhandler = XSetErrorHandler(qxt_x_errhandler);
          XUngrabKey(display, nativeKey, nativeMods, window);
          XUngrabKey(display, nativeKey, nativeMods | Mod2Mask, window); // allow numlock
          XSync(display, False);
          //XSetErrorHandler(original_x_errhandler);
          //return !error;
}
