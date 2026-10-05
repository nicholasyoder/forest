// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef BATTERY_H
#define BATTERY_H

#include <QWidget>

class battery : public QWidget
{
    Q_OBJECT

public:
    battery(QString path);

    // Re-reads sysfs and schedules a repaint.
    void refresh();
    qreal getpercentfull() const { return percentfull; }
    QString getstatus() const { return status; }
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void notifylow();

    bool sentnotification = false;
    QString pathtobatdir;
    qreal percentfull = 0;
    QString status;
};

#endif // BATTERY_H
