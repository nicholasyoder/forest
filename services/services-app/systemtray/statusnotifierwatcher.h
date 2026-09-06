// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef STATUSNOTIFIERWATCHER_H
#define STATUSNOTIFIERWATCHER_H

#include <QDBusContext>
#include <QHash>
#include <QObject>
#include <QStringList>

class QDBusServiceWatcher;

// Implements org.kde.StatusNotifierWatcher, the DBus singleton every
// StatusNotifierItem-publishing app (network applets, chat clients, media
// players, etc.) registers itself with. Nothing else provides this on a
// bare Biome session - there's no Plasma/GNOME Shell running it - so
// Forest owns it unconditionally at startup, the same reasoning as why
// Forest owns its own GlobalShortcuts portal client.
//
// This is registered directly (QDBusConnection::registerObject(path, this,
// ExportAllSlots | ExportAllSignals | ExportAllProperties), same pattern
// forest.cpp itself uses for org.forest) rather than via a
// QDBusAbstractAdaptor: QDBusContext::message() - needed here to identify
// which connection is actually calling RegisterStatusNotifierItem/Host,
// since callers can't be trusted to self-report their own bus name -
// simply does not work when QDBusContext is mixed into an adaptor. Qt's
// dispatcher only ever attaches the calling context to the object that was
// itself passed to registerObject(), never to an adaptor child, no matter
// its access specifier (confirmed the hard way: this crashed with a null
// context deref every time a real call came in, moving QDBusContext from
// protected to public on the adaptor didn't help, and it only stopped
// crashing once the interface was registered on this object directly).
//
// panel/panel-plugins/systray is the Host that reads what gets registered
// here and renders it.
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
