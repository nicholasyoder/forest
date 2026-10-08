// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HOTKEYCONFIG_H
#define HOTKEYCONFIG_H

#include <QList>
#include <QString>

#include <optional>

// A [hotkeys] action in Forest.conf of the form
// "DBUS:bus=Session,service=…,path=…,interface=…,method=…[,arg=…]".
struct DBusHotkeyAction {
    bool systemBus = false;
    QString service;
    QString path;
    QString interface;
    QString method;
    QString arg; // optional single string argument

    // Same call; an empty interface on either side matches any.
    bool sameTarget(const DBusHotkeyAction &other) const;
};

struct BuiltinHotkeyAction {
    QString description;
    DBusHotkeyAction action;
    QString note; // e.g. what must be running for the call to work
};

namespace hotkeyconfig {

// nullopt if `action` isn't a DBUS: action.
std::optional<DBusHotkeyAction> parseDBusAction(const QString &action);
QString formatDBusAction(const DBusHotkeyAction &action);

// Fixed user-facing org.forest calls; display profiles are listed separately.
QList<BuiltinHotkeyAction> builtinActions();

// Built-in actions on org.forest /org/forest/displays.
DBusHotkeyAction displayProfileAction(const QString &profileId);
DBusHotkeyAction nextDisplayProfileAction();
QString displayProfileDescription(const QString &profileName);

}

#endif // HOTKEYCONFIG_H
