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
    connection.registerObject("/org/freedesktop/Notifications", this);
    connection.registerService("org.freedesktop.Notifications");
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
