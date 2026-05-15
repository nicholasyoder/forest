// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef USERSELECTVIEW_H
#define USERSELECTVIEW_H

#include <QWidget>
#include <QHash>
#include "userlistmodel.h"

class QFrame;

class UserSelectView : public QWidget {
    Q_OBJECT
public:
    explicit UserSelectView(const QList<UserInfo> &users, QWidget *parent = nullptr);
    void highlightUser(const QString &username);

signals:
    void userSelected(const UserInfo &user);
    void otherUserRequested();

private:
    QHash<QString, QFrame *> m_tiles;
};

#endif // USERSELECTVIEW_H
