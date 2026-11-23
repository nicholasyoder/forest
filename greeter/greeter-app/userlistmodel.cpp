// SPDX-License-Identifier: LGPL-3.0-or-later

#include "userlistmodel.h"

#include <QDebug>

#include <pwd.h>

#include "loginui.h"

UserListModel::UserListModel() {
    load();
}

void UserListModel::load() {
    static const QStringList noLoginShells = {
        "/usr/sbin/nologin", "/bin/false", "/sbin/nologin"
    };

    // getpwent() goes through NSS, so SSSD/LDAP users are included.
    setpwent();
    while (const passwd *pw = getpwent()) {
        if (pw->pw_uid < 1000 || noLoginShells.contains(QString::fromLocal8Bit(pw->pw_shell)))
            continue;

        const QString username = QString::fromLocal8Bit(pw->pw_name);
        const QString homeDir = QString::fromLocal8Bit(pw->pw_dir);
        m_users.append({username, loginui::displayName(pw), homeDir,
                        loginui::faceIconPath(username, homeDir)});
    }
    endpwent();

    if (m_users.isEmpty())
        qWarning() << "No users found with UID >= 1000";
}
