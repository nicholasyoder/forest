// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef IDLEWATCHER_H
#define IDLEWATCHER_H

#include <QList>
#include <QObject>

struct LockerConfig;
class IdleNotifier;
class IdleNotification;

// One ext_idle_notification_v1 per threshold.
class IdleWatcher : public QObject {
    Q_OBJECT
public:
    enum Threshold { DisplayOff, LockedDisplayOff };
    Q_ENUM(Threshold)

    explicit IdleWatcher(QObject *parent = nullptr);
    ~IdleWatcher() override;

    // False if the compositor lacks ext-idle-notify-v1.
    bool isValid() const;
    void configure(const LockerConfig &config, bool locked);
    // Adds or drops only the while-locked threshold, so the others keep their timers.
    void setLocked(bool locked);

signals:
    void idled(IdleWatcher::Threshold threshold);
    void resumed();

private:
    void add(Threshold threshold, int timeoutMs);
    void remove(Threshold threshold);
    void notificationIdled(IdleNotification *notification);
    void notificationResumed(IdleNotification *notification);

    IdleNotifier *m_notifier;
    QList<IdleNotification *> m_notifications;
    int m_lockedDisplayOffMs = 0;
    bool m_locked = false;

    friend class IdleNotification;
};

#endif // IDLEWATCHER_H
