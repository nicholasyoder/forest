// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sessionlistmodel.h"

#include <QDir>
#include <QFileInfo>
#include <QDebug>

#include <qt6xdg/XdgDesktopFile>

SessionListModel::SessionListModel() {
    load();
}

void SessionListModel::loadDir(const QString &path, SessionType type) {
    const QDir dir(path);
    const QStringList files = dir.entryList({"*.desktop"}, QDir::Files);

    for (const QString &filename : files) {
        XdgDesktopFile file;
        if (!file.load(dir.filePath(filename)) || !file.isSuitable(true))
            continue;
        if (file.contains("TryExec") && !file.tryExec())
            continue;

        const QString name = file.name();
        const QString exec = file.value("Exec").toString();
        if (name.isEmpty() || exec.isEmpty())
            continue;

        // XdgDesktopFile returns the raw ';'-list; XDG_CURRENT_DESKTOP wants ':'.
        const QString desktopNames = file.value("DesktopNames").toString().split(';', Qt::SkipEmptyParts).join(':');
        m_sessions.append({name, exec, type, QFileInfo(filename).completeBaseName(), desktopNames});
    }
}

void SessionListModel::load() {
    loadDir("/usr/share/xsessions", SessionType::X11);
    loadDir("/usr/share/wayland-sessions", SessionType::Wayland);

    if (m_sessions.isEmpty())
        qWarning() << "No sessions found in /usr/share/xsessions/ or /usr/share/wayland-sessions/";
}
