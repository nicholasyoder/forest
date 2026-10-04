// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOGIND_H
#define LOGIND_H

#include <QDBusUnixFileDescriptor>
#include <QObject>

#include <optional>

// This session's login1 object: lock requests, sleep, and the locked/idle hints.
class Logind : public QObject {
    Q_OBJECT
public:
    explicit Logind(QObject *parent = nullptr);

    bool isValid() const { return !m_session.isEmpty(); }

    void setLockedHint(bool locked);
    void setIdleHint(bool idle);
    // A `delay` sleep inhibitor, so the lock is up before suspend.
    void takeSleepInhibitor();
    void releaseSleepInhibitor();

signals:
    void lockRequested();
    void unlockRequested();
    void prepareForSleep(bool start);

private slots:
    void onLock();
    void onUnlock();
    void onPrepareForSleep(bool start);

private:
    void callSession(const QString &method, bool value);

    QString m_session;
    QDBusUnixFileDescriptor m_inhibitor;
    std::optional<bool> m_lockedHint;
    std::optional<bool> m_idleHint;
};

#endif // LOGIND_H
