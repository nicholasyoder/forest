// SPDX-License-Identifier: LGPL-3.0-or-later

#include "locker.h"

#include "layeroverlay.h"

Locker::Locker(QObject *parent)
    : QObject(parent)
{
    connect(&m_logind, &Logind::lockRequested, &m_supervisor, &LockSupervisor::lock);
    connect(&m_logind, &Logind::unlockRequested, &m_supervisor, &LockSupervisor::unlock);
    connect(&m_logind, &Logind::prepareForSleep, this, &Locker::prepareForSleep);

    connect(&m_settings, &LockerSettings::changed, this, &Locker::applySettings);
    connect(&m_idle, &IdleWatcher::idled, this, &Locker::idled);
    connect(&m_idle, &IdleWatcher::resumed, this, &Locker::resumed);
    connect(&m_supervisor, &LockSupervisor::stateChanged, this, &Locker::lockStateChanged);
    connect(&m_supervisor, &LockSupervisor::lockFailed, this, &Locker::finishSleepLock);
    connect(&m_screenSaver, &ScreenSaver::lockRequested, &m_supervisor, &LockSupervisor::lock);
    connect(&m_screenSaver, &ScreenSaver::inhibitedChanged, &m_idle, &IdleWatcher::setInhibited);
    connect(&m_screenSaver, &ScreenSaver::activitySimulated, &m_idle, &IdleWatcher::rearm);
}

void Locker::start()
{
    // Biome doesn't wake outputs on input: undo whatever a crashed predecessor left off.
    m_power.setAll(true);
    m_logind.setIdleHint(false);
    m_logind.setLockedHint(false);
    applySettings();
    m_supervisor.restore();
}

void Locker::applySettings()
{
    const LockerConfig &config = m_settings.config();
    m_idle.configure(config);
    if (config.lockOnSuspend)
        m_logind.takeSleepInhibitor();
    else
        m_logind.releaseSleepInhibitor();
}

void Locker::idled(IdleWatcher::Threshold threshold)
{
    m_logind.setIdleHint(true);
    if (threshold == IdleWatcher::Dim) {
        showDim();
        return;
    }
    if (threshold == IdleWatcher::DisplayOff && m_settings.config().lockOnDisplayOff)
        m_supervisor.lock();
    m_power.setAll(false);
}

void Locker::resumed()
{
    m_power.setAll(true);
    hideDim();
    m_logind.setIdleHint(false);
}

void Locker::lockStateChanged(LockSupervisor::State state)
{
    m_idle.setLocked(state != LockSupervisor::Unlocked);
    m_screenSaver.setActive(state == LockSupervisor::Locked);
    m_logind.setLockedHint(state == LockSupervisor::Locked);
    if (state != LockSupervisor::Locking)
        finishSleepLock();
}

void Locker::prepareForSleep(bool start)
{
    if (!start) {
        // logind gave up waiting: keep the inhibitor for the next suspend instead of
        // letting the late lock release it.
        m_sleepLockPending = false;
        if (m_settings.config().lockOnSuspend)
            m_logind.takeSleepInhibitor();
        return;
    }
    if (!m_settings.config().lockOnSuspend || m_supervisor.state() == LockSupervisor::Locked) {
        m_logind.releaseSleepInhibitor();
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
    m_logind.releaseSleepInhibitor();
}

void Locker::showDim()
{
    if (!m_dim.isEmpty())
        return;
    // Fades only if Biome's config lists this namespace.
    m_dim = layeroverlay::showOnAllScreens(QColor(0, 0, 0, 160), LayerShellQt::Window::LayerOverlay,
                                           "forest-locker-dim", true);
}

void Locker::hideDim()
{
    for (layeroverlay *overlay : std::as_const(m_dim))
        overlay->close();
    m_dim.clear();
}
