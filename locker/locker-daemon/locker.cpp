// SPDX-License-Identifier: LGPL-3.0-or-later

#include "locker.h"

#include "logind.h"

Locker::Locker(bool useLogind, QObject *parent)
    : QObject(parent)
{
    if (useLogind) {
        m_logind = new Logind(this);
        connect(m_logind, &Logind::lockRequested, &m_supervisor, &LockSupervisor::lock);
        connect(m_logind, &Logind::unlockRequested, &m_supervisor, &LockSupervisor::unlock);
        connect(m_logind, &Logind::prepareForSleep, this, &Locker::prepareForSleep);
    }

    connect(&m_settings, &LockerSettings::changed, this, &Locker::applySettings);
    connect(&m_idle, &IdleWatcher::idled, this, &Locker::idled);
    connect(&m_idle, &IdleWatcher::resumed, this, &Locker::resumed);
    connect(&m_supervisor, &LockSupervisor::stateChanged, this, &Locker::lockStateChanged);
    connect(&m_supervisor, &LockSupervisor::lockFailed, this, &Locker::finishSleepLock);
}

void Locker::start()
{
    // Biome doesn't wake outputs on input: undo whatever a crashed predecessor left off.
    m_power.setAll(true);
    if (m_logind) {
        m_logind->setIdleHint(false);
        m_logind->setLockedHint(false);
    }
    applySettings();
    m_supervisor.restore();
}

void Locker::applySettings()
{
    const LockerConfig &config = m_settings.config();
    m_idle.configure(config, isLocked());
    if (!m_logind)
        return;
    if (config.lockOnSuspend)
        m_logind->takeSleepInhibitor();
    else
        m_logind->releaseSleepInhibitor();
}

void Locker::idled(IdleWatcher::Threshold threshold)
{
    if (m_logind)
        m_logind->setIdleHint(true);
    if (threshold == IdleWatcher::DisplayOff && m_settings.config().lockOnDisplayOff)
        m_supervisor.lock();
    m_power.setAll(false);
}

void Locker::resumed()
{
    m_power.setAll(true);
    if (m_logind)
        m_logind->setIdleHint(false);
}

void Locker::lockStateChanged(LockSupervisor::State state)
{
    m_idle.setLocked(state != LockSupervisor::Unlocked);
    if (m_logind)
        m_logind->setLockedHint(state == LockSupervisor::Locked);
    if (state != LockSupervisor::Locking)
        finishSleepLock();
}

void Locker::prepareForSleep(bool start)
{
    if (!start) {
        if (m_settings.config().lockOnSuspend)
            m_logind->takeSleepInhibitor();
        return;
    }
    if (!m_settings.config().lockOnSuspend || m_supervisor.state() == LockSupervisor::Locked) {
        m_logind->releaseSleepInhibitor();
        return;
    }
    m_sleepLockPending = true;
    m_supervisor.lock();
}

void Locker::finishSleepLock()
{
    if (!m_sleepLockPending)
        return;
    m_sleepLockPending = false;
    m_logind->releaseSleepInhibitor();
}
