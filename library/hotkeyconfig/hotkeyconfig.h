// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HOTKEYCONFIG_H
#define HOTKEYCONFIG_H

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

namespace hotkeyconfig {

// nullopt if `action` isn't a DBUS: action.
std::optional<DBusHotkeyAction> parseDBusAction(const QString &action);
QString formatDBusAction(const DBusHotkeyAction &action);

// Built-in actions on org.forest /org/forest/displays.
DBusHotkeyAction displayProfileAction(const QString &profileId);
DBusHotkeyAction nextDisplayProfileAction();
QString displayProfileDescription(const QString &profileName);

}

#endif // HOTKEYCONFIG_H
