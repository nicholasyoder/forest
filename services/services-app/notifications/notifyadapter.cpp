// SPDX-License-Identifier: LGPL-3.0-or-later

#include "notifyadapter.h"
#include "notify.h"

notifyadapter::notifyadapter(notify *parent) : QDBusAbstractAdaptor(parent), service(parent)
{
    setAutoRelaySignals(true);
}

QStringList notifyadapter::GetCapabilities()
{
    return {"body", "body-markup", "actions"};
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
    if (!service->close_popup(id, notify::ClosedByCall))
        sendErrorReply(QDBusError::InvalidArgs, "No notification with that id");
}

uint notifyadapter::Notify(const QString &app_name, uint replaces_id, const QString &app_icon, const QString &summary,
                           const QString &body, const QStringList &actions, const QVariantMap &hints, int expire_timeout)
{
    return service->show_notification(app_name, replaces_id, app_icon, summary, body, actions, hints, expire_timeout);
}
