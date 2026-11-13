// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef NOTIFY_H
#define NOTIFY_H

#include <QWidget>
#include <QIcon>
#include <QVariantMap>
#include <QDebug>
#include <QtDBus>
#include <QApplication>

#include "notifyadapter.h"
#include "notifypopup.h"

class notify : public QObject
{
    Q_OBJECT

public:
    // NotificationClosed reasons from the freedesktop spec.
    enum CloseReason : uint { Expired = 1, Dismissed = 2, ClosedByCall = 3 };

    notify();
    ~notify();

    void setup();

    uint show_notification(const QString &app_name, uint replaces_id, const QString &app_icon, const QString &summary,
                           const QString &body, const QStringList &actions, const QVariantMap &hints, int expire_timeout);
    void close_popup(uint id, uint reason);

signals:
    void NotificationClosed(uint id, uint reason);
    void ActionInvoked(uint id, const QString &action_key);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void remove_popup(uint id);
    void restack();
    void show_held();

    QHash<uint, notifypopup*> popuphash;
    QList<notifypopup*> stack; // oldest first, nearest the screen corner
    bool held = false; // forest's startup overlay is up; popups would map over it
    uint next_id = 1;
};
#endif // NOTIFY_H
