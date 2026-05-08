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

void notify::notifyslot(
        const QString &app_name,
        uint replaces_id,
        const QString &app_icon,
        const QString &summary,
        const QString &body,
        const QStringList &actions,
        const QVariantMap &hints,
        int expire_timeout,
        uint id)
{
    Q_UNUSED(actions)
    Q_UNUSED(hints);

    closeslot(replaces_id);

    notifypopup *npop = new notifypopup(app_name, summary, body, app_icon, expire_timeout, id);
    connect(npop, &notifypopup::readyToClose, this, &notify::closeslot);
    popuphash[id] = npop;
    npop->show();
}

void notify::closeslot(uint id){
    if(popuphash.contains(id) && popuphash[id]){
        popuphash[id]->close();
        popuphash[id]->deleteLater();
        popuphash.remove(id);
    }
}
