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

#include "animwidget.h"



animwidget::animwidget()
{
    //this->setWindowOpacity(0.0);

    Qt::WindowFlags flags;
    flags |= Qt::X11BypassWindowManagerHint;
    flags |= Qt::WindowStaysOnTopHint;
    //flags |= Qt::FramelessWindowHint;
    this->setWindowFlags(flags);

    //make it cover the entire screen
    int width = qApp->screens().first()->size().width();
    int height = qApp->screens().first()->size().height();
    this->setGeometry(0,0,width, height);
}

void animwidget::paintEvent(QPaintEvent *event)
{
    QRectF target1(0.0, 0.0, this->width(), this->height());
    QRectF source1(0.0, 0.0, backimage->width(), backimage->height());
    QPainter painter1(this);
    //painter.setOpacity(opacity);
    painter1.drawImage(target1, *backimage, source1);

    QRectF target(0.0, 0.0, this->width(), this->height());
    QRectF source(0.0, 0.0, wallpaper->width(), wallpaper->height());
    QPainter painter(this);
    painter.setOpacity(opacity);
    painter.drawImage(target, *wallpaper, source);
}

//grab screenshot
void animwidget::getimage()
{
    QSettings settings("Forest","Forest");
    *wallpaper = QImage(settings.value("desktop-wallpaper", "").toString());
    *backimage = QPixmap::grabWindow(QApplication::desktop()->winId()).toImage();

    //make it cover the entire screen
    //int width = qApp->desktop()->size().width();
    //int height = qApp->desktop()->size().height();
    //this->setGeometry(0,0,width, height);

    this->repaint();
    //this->setWindowOpacity(1.0);
}

void animwidget::start()
{
    connect(t, SIGNAL(timeout()), this, SLOT(fadein()));
    t->start(40);
}

void animwidget::blackout()
{
    delete t;
    t = new QTimer;
    connect(t, SIGNAL(timeout()), this, SLOT(fadeblack()));
    t->start(40);
}

void animwidget::stop()
{
    delete t;
    t = new QTimer;
    connect(t, SIGNAL(timeout()), this, SLOT(fadeout()));
    t->start(40);
}

void animwidget::fadein()
{
    if (opacity < 1.0)
    {
        opacity = opacity + 0.05;
        this->update();
    }
    else
    {
        t->stop();
        delete backimage;
        delete wallpaper;
        delete this;
    }
}

void animwidget::fadeblack()
{
    if (opacity < 1.0)
    {
        opacity = opacity + 0.05;
        this->update();
    }
    else
    {
        t->stop();
    }
}

void animwidget::fadeout()
{
    if (opacity > 0.0)
    {
        opacity = opacity - 0.05;
        this->update();
    }
    else
    {
        t->stop();
        this->close();
    }
}
