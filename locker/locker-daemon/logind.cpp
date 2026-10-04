// SPDX-License-Identifier: LGPL-3.0-or-later

#include "logind.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QDebug>

#include <unistd.h>

namespace {

const QString kService = QStringLiteral("org.freedesktop.login1");
const QString kManagerPath = QStringLiteral("/org/freedesktop/login1");
const QString kManagerIface = QStringLiteral("org.freedesktop.login1.Manager");
const QString kSessionIface = QStringLiteral("org.freedesktop.login1.Session");

} // namespace

Logind::Logind(QObject *parent)
    : QObject(parent)
{
    QDBusConnection bus = QDBusConnection::systemBus();

    // Signals arrive on the real session path, never session/self.
    QByteArray sessionId = qgetenv("XDG_SESSION_ID");
    QDBusMessage call = sessionId.isEmpty()
        ? QDBusMessage::createMethodCall(kService, kManagerPath, kManagerIface, "GetSessionByPID")
              << uint(getpid())
        : QDBusMessage::createMethodCall(kService, kManagerPath, kManagerIface, "GetSession")
              << QString::fromLocal8Bit(sessionId);
    QDBusReply<QDBusObjectPath> reply = bus.call(call);
    if (!reply.isValid()) {
        qWarning() << "logind: can't resolve this session:" << reply.error().message();
        return;
    }
    m_session = reply.value().path();

    bus.connect(kService, m_session, kSessionIface, "Lock", this, SLOT(onLock()));
    bus.connect(kService, m_session, kSessionIface, "Unlock", this, SLOT(onUnlock()));
    bus.connect(kService, kManagerPath, kManagerIface, "PrepareForSleep", this, SLOT(onPrepareForSleep(bool)));
}

void Logind::setLockedHint(bool locked)
{
    if (m_lockedHint == locked)
        return;
    m_lockedHint = locked;
    callSession("SetLockedHint", locked);
}

void Logind::setIdleHint(bool idle)
{
    if (m_idleHint == idle)
        return;
    m_idleHint = idle;
    callSession("SetIdleHint", idle);
}

void Logind::takeSleepInhibitor()
{
    if (!isValid() || m_inhibitor.isValid())
        return;
    QDBusMessage call = QDBusMessage::createMethodCall(kService, kManagerPath, kManagerIface, "Inhibit")
        << QStringLiteral("sleep") << QStringLiteral("Forest Locker")
        << QStringLiteral("Lock the screen before suspend") << QStringLiteral("delay");
    QDBusReply<QDBusUnixFileDescriptor> reply = QDBusConnection::systemBus().call(call);
    if (reply.isValid())
        m_inhibitor = reply.value();
    else
        qWarning() << "logind: Inhibit failed:" << reply.error().message();
}

void Logind::releaseSleepInhibitor()
{
    // Closing the last copy of the fd releases it.
    m_inhibitor = QDBusUnixFileDescriptor();
}

void Logind::onLock()
{
    emit lockRequested();
}

void Logind::onUnlock()
{
    emit unlockRequested();
}

void Logind::onPrepareForSleep(bool start)
{
    emit prepareForSleep(start);
}

void Logind::callSession(const QString &method, bool value)
{
    if (!isValid())
        return;
    QDBusMessage call = QDBusMessage::createMethodCall(kService, m_session, kSessionIface, method) << value;
    auto *watcher = new QDBusPendingCallWatcher(QDBusConnection::systemBus().asyncCall(call), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [method](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<> reply = *watcher;
        if (reply.isError())
            qWarning() << "logind:" << method << "failed:" << reply.error().message();
        watcher->deleteLater();
    });
}
