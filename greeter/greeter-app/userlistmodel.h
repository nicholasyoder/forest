// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef USERLISTMODEL_H
#define USERLISTMODEL_H

#include <QList>
#include <QString>

struct UserInfo {
    QString username;
    QString displayName;
    QString homeDir;
    QString faceIconPath;
};

class UserListModel {
public:
    UserListModel();
    const QList<UserInfo>& users() const { return m_users; }

private:
    QList<UserInfo> m_users;
    void load();
};

#endif // USERLISTMODEL_H
