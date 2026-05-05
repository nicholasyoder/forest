// SPDX-License-Identifier: LGPL-3.0-or-later

#include "notifyadapter.h"

notifyadapter::notifyadapter(QObject *parent) : QDBusAbstractAdaptor(parent)
{
    this->setAutoRelaySignals(true);
}

notifyadapter::~notifyadapter()
{

}

void notifyadapter::GetServerInformation(QString &name, QString &vendor, QString &version, QString &spec_version)
{
    name = "forest-notification";
    vendor = "forest";
    version = "0.5";
    spec_version = "0.1";
}

void notifyadapter::CloseNotification(const uint &id)
{
    QMetaObject::invokeMethod(parent(), "closeslot", Q_ARG(uint, id));
}

void notifyadapter::Notify(
        const QString &app_name,
        const uint replaces_id,
        const QString &app_icon,
        const QString &summary,
        const QString &body,
        const QStringList &actions,
        const QVariantMap &hints,
        const int expire_timeout,
        uint &id)
{
    QMetaObject::invokeMethod(parent(), "notifyslot",
                              Q_ARG(QString, app_name),
                              Q_ARG(uint, replaces_id),
                              Q_ARG(QString, app_icon),
                              Q_ARG(QString, summary),
                              Q_ARG(QString, body),
                              Q_ARG(QStringList, actions),
                              Q_ARG(QVariantMap, hints),
                              Q_ARG(int, expire_timeout),
                              Q_ARG(uint, id) );
}
