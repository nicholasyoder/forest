// SPDX-License-Identifier: LGPL-3.0-or-later

#include "idlewatcher.h"

#include <algorithm>

#include <QDebug>
#include <QGuiApplication>
#include <QWaylandClientExtension>
#include <qguiapplication_platform.h>

#include "qwayland-ext-idle-notify-v1.h"

class IdleNotifier : public QWaylandClientExtensionTemplate<IdleNotifier>, public QtWayland::ext_idle_notifier_v1 {
public:
    IdleNotifier() : QWaylandClientExtensionTemplate<IdleNotifier>(1) { initialize(); }
};

class IdleNotification : public QtWayland::ext_idle_notification_v1 {
public:
    IdleNotification(IdleWatcher *watcher, IdleWatcher::Threshold threshold, int timeoutMs,
                     ::ext_idle_notification_v1 *object)
        : QtWayland::ext_idle_notification_v1(object), m_watcher(watcher), m_threshold(threshold)
        , m_timeoutMs(timeoutMs) {}
    ~IdleNotification() override { destroy(); }

    IdleWatcher::Threshold threshold() const { return m_threshold; }
    int timeoutMs() const { return m_timeoutMs; }
    bool isIdle() const { return m_idle; }

protected:
    void ext_idle_notification_v1_idled() override
    {
        m_idle = true;
        m_watcher->notificationIdled(this);
    }

    void ext_idle_notification_v1_resumed() override
    {
        m_idle = false;
        m_watcher->notificationResumed(this);
    }

private:
    IdleWatcher *m_watcher;
    IdleWatcher::Threshold m_threshold;
    int m_timeoutMs;
    bool m_idle = false;
};

namespace {

// Dim warning shown this long before the displays turn off.
constexpr int DimLeadMs = 10000;

} // namespace

IdleWatcher::IdleWatcher(QObject *parent)
    : QObject(parent)
    , m_notifier(new IdleNotifier)
{
    m_notifier->setParent(this);
    if (!isValid())
        qWarning() << "Compositor lacks ext-idle-notify-v1: idle actions disabled";
}

IdleWatcher::~IdleWatcher()
{
    qDeleteAll(m_notifications);
}

bool IdleWatcher::isValid() const
{
    return m_notifier->isActive();
}

void IdleWatcher::configure(const LockerConfig &config)
{
    m_config = config;
    sync();
}

void IdleWatcher::setLocked(bool locked)
{
    m_locked = locked;
    sync();
}

void IdleWatcher::setInhibited(bool inhibited)
{
    m_inhibited = inhibited;
    sync();
}

void IdleWatcher::rearm()
{
    // remove() emits resumed() once the last idle notification goes.
    while (!m_notifications.isEmpty())
        remove(m_notifications.last());
    sync();
}

int IdleWatcher::timeoutFor(Threshold threshold) const
{
    switch (threshold) {
    case Dim:
        if (m_locked || m_inhibited || !m_config.dimBeforeDisplayOff || m_config.displayOffMs() <= DimLeadMs)
            return 0;
        return m_config.displayOffMs() - DimLeadMs;
    case DisplayOff:
        // Bus inhibitors can't tell whether their window is visible, so the lock overrides them.
        return !m_inhibited || m_locked ? m_config.displayOffMs() : 0;
    case LockedDisplayOff:
        return m_locked ? m_config.lockedDisplayOffMs() : 0;
    }
    return 0;
}

void IdleWatcher::sync()
{
    for (Threshold threshold : {Dim, DisplayOff, LockedDisplayOff}) {
        int timeoutMs = timeoutFor(threshold);
        auto it = std::find_if(m_notifications.begin(), m_notifications.end(),
                               [threshold](IdleNotification *n) { return n->threshold() == threshold; });
        if (it != m_notifications.end()) {
            if ((*it)->timeoutMs() == timeoutMs)
                continue;
            remove(*it);
        }
        add(threshold, timeoutMs);
    }
}

void IdleWatcher::add(Threshold threshold, int timeoutMs)
{
    if (timeoutMs <= 0 || !isValid())
        return;
    auto *app = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    if (!app || !app->seat())
        return;
    m_notifications.append(new IdleNotification(this, threshold, timeoutMs,
                                                m_notifier->get_idle_notification(timeoutMs, app->seat())));
}

void IdleWatcher::remove(IdleNotification *notification)
{
    m_notifications.removeOne(notification);
    bool wasIdle = notification->isIdle();
    delete notification;
    // Its resumed would never arrive; any still-idle notification will deliver one.
    if (wasIdle && std::none_of(m_notifications.begin(), m_notifications.end(),
                                [](IdleNotification *n) { return n->isIdle(); }))
        emit resumed();
}

void IdleWatcher::notificationIdled(IdleNotification *notification)
{
    emit idled(notification->threshold());
}

void IdleWatcher::notificationResumed(IdleNotification *)
{
    emit resumed();
}
