// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SESSIONLISTMODEL_H
#define SESSIONLISTMODEL_H

#include <QList>
#include <QString>

enum class SessionType { Wayland, X11 };

struct SessionInfo {
    QString name;
    QString exec;
    SessionType type;
};

class SessionListModel {
public:
    SessionListModel();
    const QList<SessionInfo>& sessions() const { return m_sessions; }

private:
    QList<SessionInfo> m_sessions;
    void load();
    void loadDir(const QString &path, SessionType type);
};

#endif // SESSIONLISTMODEL_H
