// SPDX-License-Identifier: LGPL-3.0-or-later

#include "locksupervisor.h"

#include <QDebug>
#include <QFile>
#include <QStandardPaths>

#include <csignal>
#include <iterator>
#include <sys/prctl.h>
#include <unistd.h>

namespace {

constexpr int kRespawnDelaysMs[] = {0, 1000, 5000};

// Survives a forest-locker crash so the restarted daemon knows to relock.
QString markerPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation) + "/forest-locker.locked";
}

} // namespace

LockSupervisor::LockSupervisor(QObject *parent)
    : QObject(parent)
{
    m_respawnTimer.setSingleShot(true);
    connect(&m_respawnTimer, &QTimer::timeout, this, &LockSupervisor::spawn);
}

void LockSupervisor::restore()
{
    if (QFile::exists(markerPath())) {
        qInfo() << "Previous forest-locker exited while locked: relocking";
        lock();
    }
}

void LockSupervisor::lock()
{
    m_unlockPending = false;
    if (m_state != Unlocked)
        return;
    setState(Locking);
    spawn();
}

void LockSupervisor::unlock()
{
    if (m_state == Unlocked)
        return;
    // Before `locked` the lockscreen can't unlock cleanly; signal it once it reports locked.
    if (m_state == Locked && m_process && m_process->state() == QProcess::Running)
        kill(m_process->processId(), SIGUSR1);
    else
        m_unlockPending = true;
}

void LockSupervisor::spawn()
{
    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::ForwardedErrorChannel);
    m_process->setChildProcessModifier([] {
        // Undo forest-locker's blocked signals; die with forest-locker so its replacement can relock.
        sigset_t mask;
        sigemptyset(&mask);
        pthread_sigmask(SIG_SETMASK, &mask, nullptr);
        prctl(PR_SET_PDEATHSIG, SIGKILL);
    });
    connect(m_process, &QProcess::readyReadStandardOutput, this, &LockSupervisor::readOutput);
    connect(m_process, &QProcess::finished, this, &LockSupervisor::finished);
    connect(m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error != QProcess::FailedToStart)
            return;
        qWarning() << "Failed to start forest-lockscreen:" << m_process->errorString();
        m_process->deleteLater();
        m_process = nullptr;
        fail();
    });
    m_process->start("forest-lockscreen");
}

void LockSupervisor::readOutput()
{
    while (m_process->canReadLine()) {
        if (m_process->readLine().trimmed() != "locked")
            continue;
        setState(Locked);
        if (m_unlockPending) {
            m_unlockPending = false;
            kill(m_process->processId(), SIGUSR1);
        }
    }
}

void LockSupervisor::finished(int exitCode, QProcess::ExitStatus status)
{
    m_process->deleteLater();
    m_process = nullptr;

    if (status == QProcess::NormalExit && exitCode == 0) {
        m_crashes = 0;
        m_unlockPending = false;
        setState(Unlocked);
        return;
    }
    if (status == QProcess::NormalExit && exitCode == 1) {
        qWarning() << "forest-lockscreen could not take the session lock";
        fail();
        return;
    }

    int delay = kRespawnDelaysMs[qMin(m_crashes, int(std::size(kRespawnDelaysMs)) - 1)];
    ++m_crashes;
    qWarning() << "forest-lockscreen died (status" << status << "code" << exitCode << "), respawning in" << delay << "ms";
    m_respawnTimer.start(delay);
}

void LockSupervisor::fail()
{
    m_unlockPending = false;
    setState(Unlocked);
    emit lockFailed();
}

void LockSupervisor::setState(State state)
{
    if (state == m_state)
        return;
    m_state = state;
    if (state == Unlocked) {
        QFile::remove(markerPath());
    } else {
        QFile marker(markerPath());
        marker.open(QIODevice::WriteOnly);
    }
    emit stateChanged(state);
}
