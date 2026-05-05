// SPDX-License-Identifier: LGPL-3.0-or-later

#include "battery.h"

battery::battery(QString path){
    pathtobatdir = path;
    setScaledContents(true);
    setFixedWidth(10);
}

void battery::updatedata(){
    if (pathtobatdir == ""){
        QPainter painter;
        QPixmap pix(15,this->sizeHint().height());
        pix.fill(Qt::transparent);
        qreal batheight = pix.height() - 2;
        painter.begin(&pix);
        painter.setPen(Qt::white);
        painter.drawRect(4,0,6,1);
        painter.drawRect(0,1,14, int(batheight));
        painter.end();
        setPixmap(pix);
    }
    else{
        QPainter painter;
        QPixmap pix(15,this->height());
        pix.fill(Qt::transparent);

        qreal batheight = pix.height() - 2;
        painter.begin(&pix);
        painter.setPen(Qt::white);
        painter.drawRect(4,0,6,1);
        painter.drawRect(0,1,14, int(batheight));

        batheight = batheight - 1; // now we need the height of the space inside the battery

        qreal percentage = getpercentfull();
        qreal fillheight = batheight * percentage;
        int ifill = int(fillheight);

        if (percentage < 0.15){
            painter.fillRect(1, 2 + int(batheight) - ifill, 13, ifill, Qt::red);
            trynotifylow();
        }
        else{
            if (!getstatus().startsWith("Full"))
                painter.fillRect(1, 2 + int(batheight) - ifill, 13, ifill, QColor(230,150,0));
            else
                painter.fillRect(1, 2 + int(batheight) - ifill, 13, ifill, QColor(0,200,0));

            sentnotification = false;
        }

        if (getstatus().startsWith("Charging")){
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
            polygon << bottomleft << middleleft2 << middleright2 << bottomleft;

            QPainterPath myPath;
            myPath.addPolygon(polygon);
            myPath.addPolygon(polygon2);
            painter.setBrush(QColor(255,255,0));
            painter.drawPath(myPath);
        }

        painter.end();

        setPixmap(pix);
    }
}

qreal battery::getpercentfull(){
    QString fname = (QFile::exists(pathtobatdir + "/charge_full")) ?  "charge" : "energy";
    QFile file(pathtobatdir + "/"+fname+"_full");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return 0;
    QString scapacity = file.readLine();
    qreal capacity = scapacity.toDouble();
    file.close();

    QFile file2(pathtobatdir + "/"+fname+"_now");
    if (!file2.open(QIODevice::ReadOnly | QIODevice::Text))
        return 0;
    QString slevel = file2.readLine();
    qreal level = slevel.toDouble();
    file2.close();

    return level / capacity;
}

QString battery::getstatus(){
    QFile file(pathtobatdir + "/status");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "";
    QString status = file.readLine();
    file.close();
    return status;
}

void battery::trynotifylow(){
    if (!sentnotification){
        sentnotification = true;
        notifylow();
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
