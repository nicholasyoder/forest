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

    // Registers /StatusNotifierWatcher and both org.kde.StatusNotifierWatcher
    // and org.freedesktop.StatusNotifierWatcher service names on the
    // session bus - real items/hosts are split between the two namespaces
    // in practice, so this owns both rather than guessing which is needed.
    void setup();

    QStringList registeredStatusNotifierItems() const;
    bool isStatusNotifierHostRegistered() const { return !hostBusNames.isEmpty(); }
    int protocolVersion() const { return 0; }

public slots:
    // service is either a bare bus name (item lives at the default
    // "/StatusNotifierItem" path on that connection) or, if it starts with
    // "/", an object path on the calling connection - both conventions are
    // used by real-world items. The caller's actual bus identity always
    // comes from the DBus message itself (QDBusContext::message()), never
    // trusted from the string argument, so a connection can't register an
    // identifier claiming to be a different app.
    void RegisterStatusNotifierItem(const QString &service);
    void RegisterStatusNotifierHost(const QString &service);

signals:
    // Both carry the same "busName+path" identifier format used by every
    // other real StatusNotifierWatcher implementation (confirmed against
    // LXQt's), e.g. "org.example.Notifier/StatusNotifierItem" - a Host
    // reconstructs the item's DBus service+path from this one string.
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
