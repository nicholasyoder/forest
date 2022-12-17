/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2021 Nicholas Yoder
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "battery.h"

battery::battery(QString path)
{
    pathtobatdir = path;

    this->setScaledContents(true);
    this->setFixedWidth(10);
}

void battery::updatedata()
{
    if (pathtobatdir == "")
    {
        QPainter painter;
        QPixmap pix(15,this->sizeHint().height());
        pix.fill(Qt::transparent);

        qreal batheight = pix.height() - 2;
        painter.begin(&pix);
        painter.setPen(Qt::white);
        painter.drawRect(4,0,6,1);
        painter.drawRect(0,1,14, int(batheight));

        painter.end();

        this->setPixmap(pix);
    }
    else
    {
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
        //qreal percentage = counter;

        qreal fillheight = batheight * percentage;

        int ifill = int(fillheight);

        //qDebug() << "batheight: " << batheight << "fillheight: " << fillheight;



        if (percentage < 0.15)
        {
            painter.fillRect(1, 2 + int(batheight) - ifill, 13, ifill, Qt::red);
            trynotifylow();
        }
        else
        {
            if (!getstatus().startsWith("Full"))
                painter.fillRect(1, 2 + int(batheight) - ifill, 13, ifill, QColor(255,200,0));
            else
                painter.fillRect(1, 2 + int(batheight) - ifill, 13, ifill, QColor(0,200,0));

            sentnotification = false;
        }

        //qDebug() << getstatus();

        if (getstatus().startsWith("Charging"))
        {
            //qDebug() << "charging";
            qreal height = batheight + 3;
            qreal middle = height/2;

            QPointF topright1(12, 1);

            QPointF middleleft1(2, middle);
            QPointF middleright1(7,middle + 0.7);


            QPointF middleleft2(7,middle - 0.7);
            QPointF middleright2(12, middle);

            QPointF bottomleft(2,height);

            /*
             *
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
             *
             * */

            QPolygonF polygon;
            polygon << topright1 << middleleft1 << middleright1 << topright1;

            QPolygonF polygon2;
            polygon << bottomleft << middleleft2 << middleright2 << bottomleft;

            QPainterPath myPath;

            myPath.addPolygon(polygon);
            myPath.addPolygon(polygon2);
            painter.setBrush(Qt::yellow);

            painter.drawPath(myPath);
        }

        painter.end();

        this->setPixmap(pix);

        /*if (counter > 0)
        {
            counter = counter - 0.01;
        }
        else
        {
            counter = 1;
        }*/
    }
}

qreal battery::getpercentfull()
{
    QFile file(pathtobatdir + "/charge_full");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return 0;
    QString scapacity = file.readLine();
    qreal capacity = scapacity.toDouble();
    file.close();


    QFile file2(pathtobatdir + "/charge_now");
    if (!file2.open(QIODevice::ReadOnly | QIODevice::Text))
        return 0;
    QString slevel = file2.readLine();
    qreal level = slevel.toDouble();
    file2.close();

    //qDebug() << capacity << level;

    return level / capacity;
}

QString battery::getstatus()
{
    QFile file(pathtobatdir + "/status");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "";
    QString status = file.readLine();
    file.close();
    return status;
}

void battery::trynotifylow()
{
    if (!sentnotification)
    {
        sentnotification = true;
        notifylow();
    }
}

void battery::notifylow()
{
    if (QDBusConnection::sessionBus().isConnected())
    {
        QDBusInterface iface("org.freedesktop.Notifications", "/org/freedesktop/Notifications");
        if (iface.isValid())
        {
            iface.call("Notify", "Battery Monitor", uint(1234), "dialog-warning", "Battery Low", "Please connect to external power or shutdown soon", QStringList(), QVariantMap(), -1);
        }
        else
        {
            fprintf(stderr, "%s\n", qPrintable(QDBusConnection::sessionBus().lastError().message()));
        }
    }
    else
    {
        fprintf(stderr, "Cannot connect to the D-Bus session bus.\n"
                "To start it, run:\n"
                "\teval `dbus-launch --auto-syntax`\n");
    }
}
