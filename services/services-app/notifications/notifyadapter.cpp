// SPDX-License-Identifier: LGPL-3.0-or-later

#include "notifyadapter.h"
#include "notify.h"

notifyadapter::notifyadapter(notify *parent) : QDBusAbstractAdaptor(parent), service(parent)
{
    setAutoRelaySignals(true);
}

QStringList notifyadapter::GetCapabilities()
{
    // No "body-markup": FadingLabel draws clipped bodies as plain text.
    return {"body", "actions"};
}

void notifyadapter::GetServerInformation(QString &name, QString &vendor, QString &version, QString &spec_version)
{
    name = "Forest";
    vendor = "Forest";
    version = FOREST_VERSION;
    spec_version = "1.2";
}

void notifyadapter::CloseNotification(uint id)
{
    service->close_popup(id, notify::ClosedByCall);
}

uint notifyadapter::Notify(const QString &app_name, uint replaces_id, const QString &app_icon, const QString &summary,
                           const QString &body, const QStringList &actions, const QVariantMap &hints, int expire_timeout)
{
    return service->show_notification(app_name, replaces_id, app_icon, summary, body, actions, hints, expire_timeout);
}
