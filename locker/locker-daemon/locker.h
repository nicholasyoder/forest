// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOCKER_H
#define LOCKER_H

#include <QObject>

#include "displaypower.h"
#include "idlewatcher.h"
#include "lockersettings.h"
#include "locksupervisor.h"

class Logind;

// Ties idle thresholds, display power, the lockscreen and logind together.
class Locker : public QObject {
    Q_OBJECT
public:
    explicit Locker(bool useLogind, QObject *parent = nullptr);

    void start();
    bool isLocked() const { return m_supervisor.state() != LockSupervisor::Unlocked; }

private:
    void applySettings();
    void idled(IdleWatcher::Threshold threshold);
    void resumed();
    void lockStateChanged(LockSupervisor::State state);
    void prepareForSleep(bool start);
    void finishSleepLock();

    LockerSettings m_settings;
    IdleWatcher m_idle;
    DisplayPower m_power;
    LockSupervisor m_supervisor;
    Logind *m_logind = nullptr;
    bool m_sleepLockPending = false;
};

#endif // LOCKER_H
