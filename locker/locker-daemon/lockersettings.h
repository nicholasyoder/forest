// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOCKERSETTINGS_H
#define LOCKERSETTINGS_H

#include <QFileSystemWatcher>
#include <QObject>

#include "lockerconfig.h"

// LockerConfig, reloaded whenever Locker.conf changes.
class LockerSettings : public QObject {
    Q_OBJECT
public:
    explicit LockerSettings(QObject *parent = nullptr);

    const LockerConfig &config() const { return m_config; }

signals:
    void changed();

private:
    void watch();
    void reload();

    QFileSystemWatcher m_watcher;
    QString m_path;
    LockerConfig m_config;
};

#endif // LOCKERSETTINGS_H
