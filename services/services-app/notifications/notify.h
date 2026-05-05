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
    notify();
    ~notify();

    void setup();

private slots:
    void notifyslot(const QString &app_name, uint replaces_id, const QString &app_icon, const QString &summary,
                const QString &body, const QStringList &actions, const QVariantMap &hints, int expire_timeout, uint id);

    void closeslot(uint id);

private:
    QHash<uint, notifypopup*> popuphash;

};
#endif // NOTIFY_H
