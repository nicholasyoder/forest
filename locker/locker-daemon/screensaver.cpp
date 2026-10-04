// SPDX-License-Identifier: LGPL-3.0-or-later

#include "screensaver.h"

#include <algorithm>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>

namespace {

const QString Service = QStringLiteral("org.freedesktop.ScreenSaver");

} // namespace

ScreenSaver::ScreenSaver(QObject *parent)
    : QObject(parent)
    , m_watcher(QString(), QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForUnregistration)
{
    connect(&m_watcher, &QDBusServiceWatcher::serviceUnregistered, this, &ScreenSaver::ownerVanished);

    QDBusConnection bus = QDBusConnection::sessionBus();
    // Apps use either path.
    for (const QString &path : {QStringLiteral("/org/freedesktop/ScreenSaver"), QStringLiteral("/ScreenSaver")})
        bus.registerObject(path, this, QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableSignals);
    if (!bus.registerService(Service))
        qWarning() << "Couldn't own" << Service << "(another screensaver running?):" << bus.lastError().message();
}

void ScreenSaver::setActive(bool active)
{
    if (active == m_active)
        return;
    m_active = active;
    if (active)
        m_activeTimer.start();
    emit ActiveChanged(active);
}

uint ScreenSaver::Inhibit(const QString &application, const QString &reason)
{
    const QString owner = message().service();
    uint cookie = m_nextCookie++;
    qInfo() << "Idle inhibited by" << application << owner << "(" << reason << ") cookie" << cookie;

    bool wasInhibited = isInhibited();
    m_cookies.insert(cookie, owner);
    m_watcher.addWatchedService(owner);
    if (!wasInhibited)
        emit inhibitedChanged(true);
    return cookie;
}

void ScreenSaver::UnInhibit(uint cookie)
{
    // Ignore other callers' cookies.
    if (m_cookies.value(cookie) != message().service())
        return;
    qInfo() << "Idle uninhibited, cookie" << cookie;
    removeCookie(cookie);
}

void ScreenSaver::Lock()
{
    emit lockRequested();
}

void ScreenSaver::SimulateUserActivity()
{
    emit activitySimulated();
}

bool ScreenSaver::SetActive(bool active)
{
    if (active)
        emit lockRequested();
    return active;
}

uint ScreenSaver::GetActiveTime() const
{
    return m_active ? uint(m_activeTimer.elapsed() / 1000) : 0;
}

void ScreenSaver::ownerVanished(const QString &owner)
{
    const QList<uint> cookies = m_cookies.keys(owner);
    if (!cookies.isEmpty())
        qInfo() << owner << "vanished, dropping its idle inhibitors" << cookies;
    for (uint cookie : cookies)
        removeCookie(cookie);
}

void ScreenSaver::removeCookie(uint cookie)
{
    const QString owner = m_cookies.take(cookie);
    if (owner.isEmpty())
        return;
    if (std::find(m_cookies.cbegin(), m_cookies.cend(), owner) == m_cookies.cend())
        m_watcher.removeWatchedService(owner);
    if (!isInhibited())
        emit inhibitedChanged(false);
}
