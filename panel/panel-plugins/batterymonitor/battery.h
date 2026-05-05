// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef BATTERY_H
#define BATTERY_H

#include <QWidget>
#include <QLabel>
#include <QDir>
#include <QDebug>
#include <QPainter>
#include <QtDBus>
#include <QPainterPath>


class battery : public QLabel
{
    Q_OBJECT

public:
    battery(QString path);

signals:

public slots:
    void updatedata();
    qreal getpercentfull();
    QString getstatus();
    void trynotifylow();
    void notifylow();

private:
    bool sentnotification = false;
    QString pathtobatdir;
};

#endif // BATTERY_H
