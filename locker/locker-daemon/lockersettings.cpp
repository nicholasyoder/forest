// SPDX-License-Identifier: LGPL-3.0-or-later

#include "lockersettings.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>

namespace {

// Minutes as doubles so fractional values work for testing.
int minutesMs(const QSettings &settings, const char *key, double fallback)
{
    return qMax(0, int(settings.value(key, fallback).toDouble() * 60000));
}

} // namespace

bool LockerConfig::operator==(const LockerConfig &other) const
{
    return displayOffMs == other.displayOffMs && dimBeforeDisplayOff == other.dimBeforeDisplayOff
        && lockOnDisplayOff == other.lockOnDisplayOff
        && lockOnSuspend == other.lockOnSuspend
        && lockedDisplayOffMs == other.lockedDisplayOffMs;
}

LockerSettings::LockerSettings(QObject *parent)
    : QObject(parent)
    , m_path(QSettings("Forest", "Locker").fileName())
{
    // The directory too: QSettings saves by rename, which drops the file watch.
    QDir().mkpath(QFileInfo(m_path).absolutePath());
    m_watcher.addPath(QFileInfo(m_path).absolutePath());
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &LockerSettings::reload);
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &LockerSettings::reload);
    reload();
}

void LockerSettings::reload()
{
    if (QFileInfo::exists(m_path) && !m_watcher.files().contains(m_path))
        m_watcher.addPath(m_path);

    QSettings settings("Forest", "Locker");
    LockerConfig config;
    config.displayOffMs = minutesMs(settings, "display_off_minutes", 10);
    config.dimBeforeDisplayOff = settings.value("dim_before_display_off", true).toBool();
    config.lockOnDisplayOff = settings.value("lock_on_display_off", true).toBool();
    config.lockOnSuspend = settings.value("lock_on_suspend", true).toBool();
    config.lockedDisplayOffMs = minutesMs(settings, "locked_display_off_minutes", 1);

    if (config == m_config)
        return;
    m_config = config;
    emit changed();
}
