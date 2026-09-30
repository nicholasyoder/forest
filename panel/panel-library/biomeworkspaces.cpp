// SPDX-License-Identifier: LGPL-3.0-or-later

#include "biomeworkspaces.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

namespace {
constexpr char kService[] = "org.biome";
constexpr char kPath[] = "/org/biome/Workspaces";
constexpr char kInterface[] = "org.biome.Workspaces";
}

BiomeWorkspaces::BiomeWorkspaces(QObject *parent) : QObject(parent) {
    QDBusConnection bus = QDBusConnection::sessionBus();
    m_available = bus.interface()->isServiceRegistered(kService);
    if (!m_available)
        return;

    bus.connect(kService, kPath, kInterface, "WindowWorkspacesChanged",
        this, SLOT(onWindowWorkspacesChanged(QVariantMap)));

    QDBusMessage msg = QDBusMessage::createMethodCall(kService, kPath, kInterface, "GetWindowWorkspaces");
    auto *watcher = new QDBusPendingCallWatcher(bus.asyncCall(msg), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *call){
        QDBusPendingReply<QVariantMap> reply = *call;
        if (reply.isValid())
            onWindowWorkspacesChanged(reply.value());
        call->deleteLater();
    });
}

void BiomeWorkspaces::moveToplevel(const QString &identifier, int workspace) {
    QDBusMessage msg = QDBusMessage::createMethodCall(kService, kPath, kInterface, "MoveToplevelToWorkspace");
    msg << identifier << workspace;
    QDBusConnection::sessionBus().asyncCall(msg);
}

void BiomeWorkspaces::onWindowWorkspacesChanged(const QVariantMap &windowWorkspaces) {
    m_windowWorkspaces = windowWorkspaces;
    emit windowWorkspacesChanged();
}
