// SPDX-License-Identifier: LGPL-3.0-or-later

#include "statusnotifierwatcher.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusServiceWatcher>
#include <QDebug>

StatusNotifierWatcher::StatusNotifierWatcher(QObject *parent)
    : QObject(parent)
{
    serviceWatcher = new QDBusServiceWatcher(this);
    serviceWatcher->setConnection(QDBusConnection::sessionBus());
    serviceWatcher->setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
    connect(serviceWatcher, &QDBusServiceWatcher::serviceUnregistered,
            this, &StatusNotifierWatcher::onServiceUnregistered);
}

void StatusNotifierWatcher::setup()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerObject("/StatusNotifierWatcher", this,
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties))
        qCritical() << "Failed to register /StatusNotifierWatcher on DBus:" << connection.lastError().message();

    // Real items/hosts are split between the two namespaces in practice -
    // own both so nothing has to guess which one Forest answers on.
    if (!connection.registerService("org.kde.StatusNotifierWatcher"))
        qCritical() << "Failed to register org.kde.StatusNotifierWatcher on DBus:" << connection.lastError().message();
    if (!connection.registerService("org.freedesktop.StatusNotifierWatcher"))
        qCritical() << "Failed to register org.freedesktop.StatusNotifierWatcher on DBus:" << connection.lastError().message();
}

QStringList StatusNotifierWatcher::registeredStatusNotifierItems() const
{
    return identifierToBusName.keys();
}

void StatusNotifierWatcher::RegisterStatusNotifierItem(const QString &service)
{
    const QString busName = message().service();
    const QString path = service.startsWith('/') ? service : QStringLiteral("/StatusNotifierItem");
    const QString identifier = busName + path;

    if (identifierToBusName.contains(identifier))
        return;

    identifierToBusName.insert(identifier, busName);
    serviceWatcher->addWatchedService(busName);
    emit StatusNotifierItemRegistered(identifier);
}

void StatusNotifierWatcher::RegisterStatusNotifierHost(const QString &service)
{
    Q_UNUSED(service);
    const QString busName = message().service();

    if (hostBusNames.contains(busName))
        return;

    hostBusNames.append(busName);
    serviceWatcher->addWatchedService(busName);
    emit StatusNotifierHostRegistered();
}

void StatusNotifierWatcher::onServiceUnregistered(const QString &busName)
{
    serviceWatcher->removeWatchedService(busName);
    hostBusNames.removeAll(busName);

    const QStringList goneIdentifiers = identifierToBusName.keys(busName);
    for (const QString &identifier : goneIdentifiers) {
        identifierToBusName.remove(identifier);
        emit StatusNotifierItemUnregistered(identifier);
    }
}
