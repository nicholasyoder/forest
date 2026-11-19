// SPDX-License-Identifier: LGPL-3.0-or-later

#include "battery.h"

#include <QFile>
#include <QtDBus>

static QString readline(const QString &path){
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "";
    return QString(file.readLine()).trimmed();
}

battery::battery(QString path){
    pathtobatdir = path;
}

void battery::refresh(){
    QString fname = QFile::exists(pathtobatdir + "/charge_full") ? "charge" : "energy";
    full = readline(pathtobatdir + "/" + fname + "_full").toDouble();
    now = readline(pathtobatdir + "/" + fname + "_now").toDouble();
    status = readline(pathtobatdir + "/status");

    if (getpercentfull() < 0.15 && status != "Charging" && status != "Full"){
        if (!sentnotification){
            sentnotification = true;
            notifylow();
        }
    }
    else if (status == "Charging" || status == "Full" || getpercentfull() >= 0.15)
        sentnotification = false;
}

void battery::notifylow(){
    if (QDBusConnection::sessionBus().isConnected()){
        QDBusInterface iface("org.freedesktop.Notifications", "/org/freedesktop/Notifications");
        if (iface.isValid())
            {
            QDBusReply<uint> reply = iface.call("Notify", "Battery Monitor", notificationid, "dialog-warning", "Battery Low", "Connect to external power or shutdown soon", QStringList(), QVariantMap(), -1);
            if (reply.isValid())
                notificationid = reply.value();
        }
        else
            fprintf(stderr, "%s\n", qPrintable(QDBusConnection::sessionBus().lastError().message()));
    }
}
