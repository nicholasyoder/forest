// SPDX-License-Identifier: LGPL-3.0-or-later

#include "battery.h"

#include <QFile>
#include <QPainter>
#include <QPainterPath>
#include <QtDBus>

static QString readline(const QString &path){
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "";
    return QString(file.readLine()).trimmed();
}

battery::battery(QString path){
    pathtobatdir = path;
    setFixedWidth(10);
}

QSize battery::sizeHint() const {
    return QSize(10, fontMetrics().height());
}

void battery::refresh(){
    if (pathtobatdir.isEmpty())
        return;

    QString fname = QFile::exists(pathtobatdir + "/charge_full") ? "charge" : "energy";
    qreal capacity = readline(pathtobatdir + "/" + fname + "_full").toDouble();
    qreal level = readline(pathtobatdir + "/" + fname + "_now").toDouble();
    percentfull = capacity > 0 ? level / capacity : 0;
    status = readline(pathtobatdir + "/status");

    if (percentfull < 0.15){
        if (!sentnotification){
            sentnotification = true;
            notifylow();
        }
    }
    else
        sentnotification = false;

    update();
}

void battery::paintEvent(QPaintEvent *){
    QPainter painter(this);
    // Drawn on a 15px-wide canvas squeezed into the 10px widget.
    painter.scale(width() / 15.0, 1);

    qreal batheight = height() - 2;
    painter.setPen(Qt::white);
    painter.drawRect(4,0,6,1);
    painter.drawRect(0,1,14, int(batheight));

    if (pathtobatdir.isEmpty())
        return;

    batheight = batheight - 1; // height of the space inside the battery
    int ifill = int(batheight * percentfull);

    QColor fill;
    if (percentfull < 0.15)
        fill = Qt::red;
    else if (status.startsWith("Full"))
        fill = QColor(0,200,0);
    else
        fill = QColor(230,150,0);
    painter.fillRect(1, 2 + int(batheight) - ifill, 13, ifill, fill);

    if (status.startsWith("Charging")){
        qreal height = batheight + 3;
        qreal middle = height/2;

        QPointF topright1(12, 1);
        QPointF middleleft1(2, middle);
        QPointF middleright1(7,middle + 0.7);
        QPointF middleleft2(7,middle - 0.7);
        QPointF middleright2(12, middle);
        QPointF bottomleft(2,height);

        /*
         * topright:- - - - - - - - - - > /
         *                             ///
         *                          /////
         * middleleft1- - - - >  ///////  <- - \
         * middleright1- - - - - - - - - - - - /
         *
         * middleright2- - - - - - - - - - - - \
         * middleleft2- - - - - > /////// <- - /
         *                       /////
         *                      ///
         * bottomleft- - - - > /
         */

        QPolygonF polygon;
        polygon << topright1 << middleleft1 << middleright1 << topright1;
        QPolygonF polygon2;
        polygon2 << bottomleft << middleleft2 << middleright2 << bottomleft;

        QPainterPath myPath;
        myPath.addPolygon(polygon);
        myPath.addPolygon(polygon2);
        painter.setBrush(QColor(255,255,0));
        painter.drawPath(myPath);
    }
}

void battery::notifylow(){
    if (QDBusConnection::sessionBus().isConnected()){
        QDBusInterface iface("org.freedesktop.Notifications", "/org/freedesktop/Notifications");
        if (iface.isValid())
            iface.call("Notify", "Battery Monitor", uint(1234), "dialog-warning", "Battery Low", "Connect to external power or shutdown soon", QStringList(), QVariantMap(), -1);
        else
            fprintf(stderr, "%s\n", qPrintable(QDBusConnection::sessionBus().lastError().message()));
    }
}
