// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef BATTERY_H
#define BATTERY_H

#include <QString>

class battery
{
public:
    battery(QString path);

    // Re-reads sysfs.
    void refresh();
    qreal getnow() const { return now; }
    qreal getfull() const { return full; }
    qreal getpercentfull() const { return full > 0 ? now / full : 0; }
    QString getstatus() const { return status; }

private:
    void notifylow();

    bool sentnotification = false;
    QString pathtobatdir;
    qreal now = 0;
    qreal full = 0;
    QString status;
};

#endif // BATTERY_H
