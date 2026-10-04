// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCREENSAVER_H
#define SCREENSAVER_H

#include <QDBusContext>
#include <QDBusServiceWatcher>
#include <QElapsedTimer>
#include <QHash>
#include <QObject>

// org.freedesktop.ScreenSaver: idle inhibitors and lock requests from apps that
// don't use the Wayland idle-inhibit protocol.
class ScreenSaver : public QObject, protected QDBusContext {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.ScreenSaver")
public:
    explicit ScreenSaver(QObject *parent = nullptr);

    bool isInhibited() const { return !m_cookies.isEmpty(); }
    void setActive(bool active);

public slots:
    Q_SCRIPTABLE uint Inhibit(const QString &application, const QString &reason);
    Q_SCRIPTABLE void UnInhibit(uint cookie);
    Q_SCRIPTABLE void Lock();
    Q_SCRIPTABLE void SimulateUserActivity();
    Q_SCRIPTABLE bool GetActive() const { return m_active; }
    // Only activation: deactivating would unlock without authentication.
    Q_SCRIPTABLE bool SetActive(bool active);
    // Seconds active (locked), 0 if inactive.
    Q_SCRIPTABLE uint GetActiveTime() const;

signals:
    Q_SCRIPTABLE void ActiveChanged(bool active);
    void inhibitedChanged(bool inhibited);
    void lockRequested();
    void activitySimulated();

private:
    void ownerVanished(const QString &owner);
    void removeCookie(uint cookie);

    QDBusServiceWatcher m_watcher;
    QHash<uint, QString> m_cookies; // cookie -> caller's unique bus name
    uint m_nextCookie = 1;
    bool m_active = false;
    QElapsedTimer m_activeTimer;
};

#endif // SCREENSAVER_H
