// SPDX-License-Identifier: LGPL-3.0-or-later

#include "userlistmodel.h"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QSettings>
#include <QDebug>

UserListModel::UserListModel() {
    load();
}

void UserListModel::load() {
    static const QStringList noLoginShells = {
        "/usr/sbin/nologin", "/bin/false", "/sbin/nologin"
    };

    QFile passwd("/etc/passwd");
    if (!passwd.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open /etc/passwd";
        return;
    }

    QTextStream in(&passwd);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue;

        QStringList fields = line.split(':');
        if (fields.size() < 7)
            continue;

        int uid = fields[2].toInt();
        if (uid < 1000)
            continue;

        QString shell = fields[6];
        if (noLoginShells.contains(shell))
            continue;

        QString username = fields[0];
        QString homeDir = fields[5];

        QString gecos = fields[4];
        QString displayName = gecos.split(',').first().trimmed();
        if (displayName.isEmpty())
            displayName = username;

        QString faceIconPath;
        QString facePath = homeDir + "/.face";
        if (QFileInfo::exists(facePath)) {
            faceIconPath = facePath;
        } else {
            QFile accountsFile("/var/lib/AccountsService/users/" + username);
            if (accountsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream as(&accountsFile);
                while (!as.atEnd()) {
                    QString l = as.readLine();
                    if (l.startsWith("Icon=")) {
                        faceIconPath = l.mid(5).trimmed();
                        break;
                    }
                }
            }
        }

        m_users.append({username, displayName, homeDir, faceIconPath});
    }

    if (m_users.isEmpty())
        qWarning() << "No users found in /etc/passwd with UID >= 1000";
}
