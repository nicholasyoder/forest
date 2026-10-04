// SPDX-License-Identifier: LGPL-3.0-or-later

#include "idlewatcher.h"

#include <QDebug>
#include <QGuiApplication>
#include <QWaylandClientExtension>
#include <qguiapplication_platform.h>

#include "lockersettings.h"
#include "qwayland-ext-idle-notify-v1.h"

class IdleNotifier : public QWaylandClientExtensionTemplate<IdleNotifier>, public QtWayland::ext_idle_notifier_v1 {
public:
    IdleNotifier() : QWaylandClientExtensionTemplate<IdleNotifier>(1) { initialize(); }
};

class IdleNotification : public QtWayland::ext_idle_notification_v1 {
public:
    IdleNotification(IdleWatcher *watcher, IdleWatcher::Threshold threshold, ::ext_idle_notification_v1 *object)
        : QtWayland::ext_idle_notification_v1(object), m_watcher(watcher), m_threshold(threshold) {}
    ~IdleNotification() override { destroy(); }

    IdleWatcher::Threshold threshold() const { return m_threshold; }
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
    bool m_idle = false;
};

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

void IdleWatcher::configure(const LockerConfig &config, bool locked)
{
    for (Threshold threshold : {DisplayOff, LockedDisplayOff})
        remove(threshold);
    add(DisplayOff, config.displayOffMs);
    m_lockedDisplayOffMs = config.lockedDisplayOffMs;
    m_locked = locked;
    if (locked)
        add(LockedDisplayOff, m_lockedDisplayOffMs);
}

void IdleWatcher::setLocked(bool locked)
{
    if (locked == m_locked)
        return;
    m_locked = locked;
    remove(LockedDisplayOff);
    if (locked)
        add(LockedDisplayOff, m_lockedDisplayOffMs);
}

void IdleWatcher::add(Threshold threshold, int timeoutMs)
{
    if (timeoutMs <= 0 || !isValid())
        return;
    auto *app = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    if (!app || !app->seat())
        return;
    m_notifications.append(new IdleNotification(this, threshold,
                                                m_notifier->get_idle_notification(timeoutMs, app->seat())));
}

void IdleWatcher::remove(Threshold threshold)
{
    for (auto it = m_notifications.begin(); it != m_notifications.end();) {
        if ((*it)->threshold() != threshold) {
            ++it;
            continue;
        }
        // Its resumed would never arrive, leaving the displays dark.
        bool wasIdle = (*it)->isIdle();
        delete *it;
        it = m_notifications.erase(it);
        if (wasIdle)
            emit resumed();
    }
}

void IdleWatcher::notificationIdled(IdleNotification *notification)
{
    emit idled(notification->threshold());
}

void IdleWatcher::notificationResumed(IdleNotification *)
{
    emit resumed();
}
