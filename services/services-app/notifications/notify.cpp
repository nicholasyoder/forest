// SPDX-License-Identifier: LGPL-3.0-or-later

#include "notify.h"

notify::notify()
{
}

notify::~notify()
{
}

void notify::setup()
{
    new notifyadapter(this);
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerObject("/org/freedesktop/Notifications", this))
        qCritical() << "Failed to register /org/freedesktop/Notifications on DBus:" << connection.lastError().message();
    if (!connection.registerService("org.freedesktop.Notifications"))
        qCritical() << "Failed to register org.freedesktop.Notifications on DBus:" << connection.lastError().message();
}

uint notify::show_notification(
        const QString &app_name,
        uint replaces_id,
        const QString &app_icon,
        const QString &summary,
        const QString &body,
        const QStringList &actions,
        const QVariantMap &hints,
        int expire_timeout)
{
    uint id;
    if (replaces_id != 0 && popuphash.contains(replaces_id)) {
        id = replaces_id;
        remove_popup(id); // a replaced notification isn't "closed", so no signal
    } else {
        id = next_id++;
        if (next_id == 0)
            next_id = 1; // 0 is reserved by the spec
    }

    bool resident = hints.value("resident").toBool();
    notifypopup *npop = new notifypopup(app_name, summary, body, app_icon, actions, expire_timeout, id);
    connect(npop, &notifypopup::readyToClose, this, &notify::close_popup);
    connect(npop, &notifypopup::actionInvoked, this, [this, resident](uint id, const QString &key) {
        emit ActionInvoked(id, key);
        if (!resident)
            close_popup(id, Dismissed);
    });
    popuphash[id] = npop;
    npop->show();
    return id;
}

void notify::close_popup(uint id, uint reason){
    if (!popuphash.contains(id))
        return;
    remove_popup(id);
    emit NotificationClosed(id, reason);
}

void notify::remove_popup(uint id){
    notifypopup *npop = popuphash.take(id);
    if (npop) {
        npop->close();
        npop->deleteLater();
    }
}
