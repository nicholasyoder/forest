// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOCKSUPERVISOR_H
#define LOCKSUPERVISOR_H

#include <QObject>
#include <QProcess>
#include <QTimer>

// Runs forest-lockscreen and respawns it if it crashes while the session should be locked
// (the compositor keeps the session locked and lets the replacement take over). Gives up
// only if it keeps crashing before its lock request reached the compositor.
class LockSupervisor : public QObject {
    Q_OBJECT
public:
    // Unlocking: unlock signalled, lockscreen not yet reaped.
    enum State { Unlocked, Locking, Locked, Unlocking };
    Q_ENUM(State)

    explicit LockSupervisor(QObject *parent = nullptr);

    State state() const { return m_state; }
    // Relocks if a previous forest-locker died while locked.
    void restore();
    void lock();
    void unlock();

signals:
    void stateChanged(LockSupervisor::State state);
    // Another locker holds the lock, or the lockscreen couldn't run.
    void lockFailed();

private:
    void spawn();
    void sendUnlock();
    void readOutput();
    void finished(int exitCode, QProcess::ExitStatus status);
    void fail();
    void setState(State state);

    State m_state = Unlocked;
    QProcess *m_process = nullptr;
    QTimer m_respawnTimer;
    int m_crashes = 0;
    bool m_processLocked = false;
    bool m_unlockPending = false;
    bool m_relockPending = false;
    // The compositor may hold a lock for us: never give up respawning.
    bool m_requested = false;
};

#endif // LOCKSUPERVISOR_H
