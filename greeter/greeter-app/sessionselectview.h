// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SESSIONSELECTVIEW_H
#define SESSIONSELECTVIEW_H

#include <QWidget>
#include <QList>
#include "sessionlistmodel.h"

class QFrame;

class SessionSelectView : public QWidget {
    Q_OBJECT
public:
    explicit SessionSelectView(const QList<SessionInfo> &sessions, QWidget *parent = nullptr);
    void highlightSession(int index);

signals:
    void sessionSelected(int index);
    void cancelled();

private:
    QList<QFrame *> m_tiles;
};

#endif // SESSIONSELECTVIEW_H
