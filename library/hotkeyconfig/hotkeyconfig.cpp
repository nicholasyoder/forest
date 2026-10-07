// SPDX-License-Identifier: LGPL-3.0-or-later

#include "hotkeyconfig.h"

#include <QHash>
#include <QStringList>

static const QString kPrefix = QStringLiteral("DBUS:");

bool DBusHotkeyAction::sameTarget(const DBusHotkeyAction &other) const{
    return systemBus == other.systemBus && service == other.service && path == other.path
        && method == other.method && arg == other.arg
        && (interface.isEmpty() || other.interface.isEmpty() || interface == other.interface);
}

namespace hotkeyconfig {

std::optional<DBusHotkeyAction> parseDBusAction(const QString &action){
    if (!action.startsWith(kPrefix)) return std::nullopt;

    QHash<QString, QString> options;
    for (const QString &s : action.mid(kPrefix.size()).split(',')){
        const QStringList keyvalue = s.split('=');
        if (keyvalue.size() == 2) options[keyvalue.first()] = keyvalue.last();
    }

    DBusHotkeyAction a;
    a.systemBus = options.value("bus") == "System";
    a.service = options.value("service");
    a.path = options.value("path");
    a.interface = options.value("interface");
    a.method = options.value("method");
    a.arg = options.value("arg");
    return a;
}

QString formatDBusAction(const DBusHotkeyAction &a){
    QStringList parts = {
        "bus=" + QString(a.systemBus ? "System" : "Session"),
        "service=" + a.service,
        "path=" + a.path,
        "interface=" + a.interface,
        "method=" + a.method,
    };
    if (!a.arg.isEmpty()) parts << "arg=" + a.arg;
    return kPrefix + parts.join(',');
}

static DBusHotkeyAction displaysAction(const QString &method, const QString &arg = QString()){
    DBusHotkeyAction a;
    a.service = "org.forest";
    a.path = "/org/forest/displays";
    a.interface = "org.forest.displays";
    a.method = method;
    a.arg = arg;
    return a;
}

DBusHotkeyAction displayProfileAction(const QString &profileId){
    return displaysAction("applyProfile", profileId);
}

DBusHotkeyAction nextDisplayProfileAction(){
    return displaysAction("nextProfile");
}

QString displayProfileDescription(const QString &profileName){
    return "Display profile: " + profileName;
}

}
