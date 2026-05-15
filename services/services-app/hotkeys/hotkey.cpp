// SPDX-License-Identifier: LGPL-3.0-or-later

#include "hotkey.h"
#include "xcbutills.h"

#include <X11/Xlib.h>
#include "qxt/keymapper_x11.h"

globalhotkey::globalhotkey(const QKeySequence& sequence, HK_Type type) : hotkey_type(type){
    setShortcut(sequence);
    qDebug() << "Add Global Hotkey: " << sequence.toString();
}

globalhotkey::~globalhotkey(){
    unsetShortcut();
}

void globalhotkey::setDbusInfo(QString service, QString path,QString interface, QString method, QString bus){
    dbusservice = service;
    dbuspath = path;
    dbusinterface = interface;
    dbusmethod = method;
    dbusbus = bus;
}

void globalhotkey::exec(){
    if(hotkey_type == Type_Exec){
        QProcess::startDetached(shcommand);
    }
    else{
        if (dbusbus == "Session" || dbusbus == ""){
            QDBusInterface dbus(dbusservice, dbuspath, dbusinterface, QDBusConnection::sessionBus());
            if (!dbus.isValid()){
                qDebug() << "QDBusInterface is not valid!";
                return ;
            }
            dbus.call(dbusmethod);
        }
        else{
            QDBusInterface dbus(dbusservice, dbuspath, dbusinterface, QDBusConnection::systemBus());
            if (!dbus.isValid()){
                qDebug() << "QDBusInterface is not valid!";
                return ;
            }
            dbus.call(dbusmethod);
        }
    }
}

void globalhotkey::XcbEventFilter(xcb_generic_event_t *event){
    if (event->response_type == XCB_KEY_PRESS){
        xcb_key_press_event_t *keyevent = (xcb_key_press_event_t*)event;
        xcb_keycode_t nativeKey = keyevent->detail;
        quint32 nativeMods = keyevent->state & (ShiftMask | ControlMask | Mod1Mask | Mod4Mask);

        lastkeypressed = nativeKey;
        if(nativeKey == keycode && nativeMods == modmask)
            exec();
    }
    else if (event->response_type == XCB_KEY_RELEASE){
        // This supports Meta key only
        xcb_key_release_event_t *keyevent = (xcb_key_release_event_t*)event;
        xcb_keycode_t nativeKey = keyevent->detail;

        if((nativeKey == 133 || nativeKey == 134) && keycode == 133 && lastkeypressed == nativeKey)
            exec();
    }
}

void globalhotkey::setShortcut(const QKeySequence& shortcut){
    Qt::KeyboardModifiers allMods = Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier;
    Qt::Key key = shortcut.isEmpty() ? Qt::Key(0) : Qt::Key((shortcut[0] ^ allMods) & shortcut[0]);
    Qt::KeyboardModifiers mods = shortcut.isEmpty() ? Qt::KeyboardModifiers() : Qt::KeyboardModifiers(shortcut[0] & allMods);

    if(key == Qt::Key_Meta){
        keycode = 133;
        modmask = XCB_MOD_MASK_ANY;
        registerShortcut(134, modmask);
    }
    else{
        keycode = nativeKeycode(key);
        modmask = nativeModifiers(mods);
    }

    registerShortcut(keycode, modmask);
}

void globalhotkey::pause(){
    if (keycode == 133)
        unregisterShortcut(134, modmask);
    unregisterShortcut(keycode, modmask);
}

void globalhotkey::resume(){
    if (keycode == 133)
        registerShortcut(134, modmask);
    registerShortcut(keycode, modmask);
}

void globalhotkey::unsetShortcut(){
    if(keycode == 133)
        unregisterShortcut(134, modmask);

    unregisterShortcut(keycode, modmask);
    keycode = nativeKeycode(Qt::Key(0));
    modmask = nativeModifiers(Qt::KeyboardModifiers());
}

quint32 globalhotkey::nativeModifiers(Qt::KeyboardModifiers modifiers){
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

quint32 globalhotkey::nativeKeycode(Qt::Key key){
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

    Display* display = Xcbutills::display();
    return XKeysymToKeycode(display, keysym);
}

void globalhotkey::registerShortcut(quint32 nativeKey, quint32 nativeMods){
    xcb_window_t window = Xcbutills::root_window();
    xcb_grab_key(Xcbutills::conn, True, window, nativeMods, nativeKey, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    xcb_grab_key(Xcbutills::conn, True, window, nativeMods | Mod2Mask, nativeKey, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
}

void globalhotkey::unregisterShortcut(quint32 nativeKey, quint32 nativeMods){
    xcb_window_t window = Xcbutills::root_window();
    xcb_ungrab_key(Xcbutills::conn, nativeKey, window, nativeMods);
    xcb_ungrab_key(Xcbutills::conn, nativeKey, window, nativeMods | Mod2Mask);
}
