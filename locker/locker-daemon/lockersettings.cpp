// SPDX-License-Identifier: LGPL-3.0-or-later

#include "lockersettings.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>

LockerSettings::LockerSettings(QObject *parent)
    : QObject(parent)
    , m_path(QSettings("Forest", "Locker").fileName())
{
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &LockerSettings::reload);
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &LockerSettings::reload);
    reload();
}

// QSettings saves by rename, which drops the file's watch: re-add it on every change.
// Until the file exists, watch its directory for it instead.
void LockerSettings::watch()
{
    const QString dir = QFileInfo(m_path).absolutePath();
    if (QFileInfo::exists(m_path)) {
        if (!m_watcher.files().contains(m_path))
            m_watcher.addPath(m_path);
        if (m_watcher.directories().contains(dir))
            m_watcher.removePath(dir);
    } else if (!m_watcher.directories().contains(dir)) {
        QDir().mkpath(dir);
        m_watcher.addPath(dir);
    }
}

void LockerSettings::reload()
{
    watch();
    LockerConfig config = LockerConfig::load();
    if (config == m_config)
        return;
    m_config = config;
    emit changed();
}
