// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef IDLEWATCHER_H
#define IDLEWATCHER_H

#include <QList>
#include <QObject>

#include "lockersettings.h"

class IdleNotifier;
class IdleNotification;

// One ext_idle_notification_v1 per threshold.
class IdleWatcher : public QObject {
    Q_OBJECT
public:
    enum Threshold { Dim, DisplayOff, LockedDisplayOff };
    Q_ENUM(Threshold)

    explicit IdleWatcher(QObject *parent = nullptr);
    ~IdleWatcher() override;

    // False if the compositor lacks ext-idle-notify-v1.
    bool isValid() const;
    void configure(const LockerConfig &config);
    void setLocked(bool locked);
    // org.freedesktop.ScreenSaver inhibitors: suspend the unlocked thresholds.
    void setInhibited(bool inhibited);

signals:
    void idled(IdleWatcher::Threshold threshold);
    void resumed();

private:
    int timeoutFor(Threshold threshold) const;
    // Re-arms only thresholds whose timeout changed, so the others keep their timers.
    void sync();
    void add(Threshold threshold, int timeoutMs);
    void remove(IdleNotification *notification);
    void notificationIdled(IdleNotification *notification);
    void notificationResumed(IdleNotification *notification);

    IdleNotifier *m_notifier;
    QList<IdleNotification *> m_notifications;
    LockerConfig m_config;
    bool m_locked = false;
    bool m_inhibited = false;

    friend class IdleNotification;
};

#endif // IDLEWATCHER_H
