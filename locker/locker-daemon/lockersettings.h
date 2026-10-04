// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOCKERSETTINGS_H
#define LOCKERSETTINGS_H

#include <QFileSystemWatcher>
#include <QObject>

// Timeouts in ms; 0 = never.
struct LockerConfig {
    int displayOffMs = 0;
    bool lockOnDisplayOff = false;
    bool lockOnSuspend = false;
    int lockedDisplayOffMs = 0;

    bool operator==(const LockerConfig &other) const;
    bool operator!=(const LockerConfig &other) const { return !(*this == other); }
};

// QSettings("Forest", "Locker"), reloaded whenever the file changes.
class LockerSettings : public QObject {
    Q_OBJECT
public:
    explicit LockerSettings(QObject *parent = nullptr);

    const LockerConfig &config() const { return m_config; }

signals:
    void changed();

private:
    void reload();

    QFileSystemWatcher m_watcher;
    QString m_path;
    LockerConfig m_config;
};

#endif // LOCKERSETTINGS_H
