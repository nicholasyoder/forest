// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sessionlistmodel.h"

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>

SessionListModel::SessionListModel() {
    load();
}

void SessionListModel::loadDir(const QString &path, SessionType type) {
    QDir dir(path);
    if (!dir.exists())
        return;
    const QStringList files = dir.entryList({"*.desktop"}, QDir::Files);

    for (const QString &filename : files) {
        QFile file(dir.filePath(filename));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;

        QString name, exec;
        bool inDesktopEntry = false;
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line == "[Desktop Entry]") {
                inDesktopEntry = true;
                continue;
            }
            if (line.startsWith('[')) {
                inDesktopEntry = false;
                continue;
            }
            if (!inDesktopEntry)
                continue;
            if (line.startsWith("Name="))
                name = line.mid(5);
            else if (line.startsWith("Exec="))
                exec = line.mid(5);
        }

        if (!name.isEmpty() && !exec.isEmpty())
            m_sessions.append({name, exec, type});
    }
}

void SessionListModel::load() {
    loadDir("/usr/share/xsessions", SessionType::X11);
    loadDir("/usr/share/wayland-sessions", SessionType::Wayland);

    if (m_sessions.isEmpty())
        qWarning() << "No sessions found in /usr/share/xsessions/ or /usr/share/wayland-sessions/";
}
