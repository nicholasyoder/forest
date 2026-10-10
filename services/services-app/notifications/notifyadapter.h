// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef NOTIFYADAPTER_H
#define NOTIFYADAPTER_H

#include <QObject>
#include <QtDBus>

class notify;

class notifyadapter : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Notifications")

public:
    notifyadapter(notify *parent);

public slots:
    QStringList GetCapabilities();
    void GetServerInformation(QString &name, QString &vendor, QString &version, QString &spec_version);
    void CloseNotification(uint id);
    uint Notify(const QString &app_name, uint replaces_id, const QString &app_icon, const QString &summary,
                const QString &body, const QStringList &actions, const QVariantMap &hints, int expire_timeout);

signals:
    // Relayed from notify by setAutoRelaySignals().
    void NotificationClosed(uint id, uint reason);
    void ActionInvoked(uint id, const QString &action_key);

private:
    notify *service;
};

#endif // NOTIFYADAPTER_H
