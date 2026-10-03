// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef STATUSNOTIFIERWATCHER_H
#define STATUSNOTIFIERWATCHER_H

#include <QDBusContext>
#include <QHash>
#include <QObject>
#include <QStringList>

class QDBusServiceWatcher;

// org.kde.StatusNotifierWatcher; panel/panel-plugins/systray is the host.
// QDBusContext only works on the object passed to registerObject(), not an adaptor.
class StatusNotifierWatcher : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.StatusNotifierWatcher")

    Q_PROPERTY(QStringList RegisteredStatusNotifierItems READ registeredStatusNotifierItems)
    Q_PROPERTY(bool IsStatusNotifierHostRegistered READ isStatusNotifierHostRegistered)
    Q_PROPERTY(int ProtocolVersion READ protocolVersion)

public:
    explicit StatusNotifierWatcher(QObject *parent = nullptr);

    // Owns both the org.kde and org.freedesktop names; real items use either.
    void setup();

    QStringList registeredStatusNotifierItems() const;
    bool isStatusNotifierHostRegistered() const { return !hostBusNames.isEmpty(); }
    int protocolVersion() const { return 0; }

public slots:
    // service: bus name (default /StatusNotifierItem path) or an object path on
    // the caller. The bus name always comes from the message, never the argument.
    void RegisterStatusNotifierItem(const QString &service);
    void RegisterStatusNotifierHost(const QString &service);

signals:
    // Identifier is "busName/path", e.g. "org.example.Notifier/StatusNotifierItem".
    void StatusNotifierItemRegistered(const QString &service);
    void StatusNotifierItemUnregistered(const QString &service);
    void StatusNotifierHostRegistered();

private slots:
    // Cleans up after a crashed/exited app that never explicitly
    // unregistered - without this, its icon would stay in the tray forever.
    void onServiceUnregistered(const QString &busName);

private:
    QHash<QString, QString> identifierToBusName; // "busName+path" -> owning unique bus name
    QStringList hostBusNames;
    QDBusServiceWatcher *serviceWatcher;
};

#endif // STATUSNOTIFIERWATCHER_H
