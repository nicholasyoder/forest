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

#include "imagewidget.h"

#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>

imagewidget::imagewidget()
{
    this->setWindowOpacity(0.0);
    this->setWindowFlags(Qt::X11BypassWindowManagerHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    //make widget the size of the screen
    int width = qApp->primaryScreen()->size().width();
    int height = qApp->primaryScreen()->size().height();
    this->setGeometry(0,0,width, height);
}

void imagewidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setOpacity(opacity);
    painter.fillRect(0,0,width(),height(), QColor(0,0,0,255));
}

void imagewidget::start()
{
    *backimage = qApp->primaryScreen()->grabWindow(QApplication::desktop()->winId()).toImage();

    this->repaint();

    this->setWindowOpacity(1.0);

    connect(animtimer, SIGNAL(timeout()), this, SLOT(fade()));
    animstarttime = QDateTime::currentMSecsSinceEpoch();
    animtime = 100;
    animtimer->start(0);
}

void imagewidget::blackout()
{
    delete t;
    t = new QTimer;
    connect(t, SIGNAL(timeout()), this, SLOT(fadeblack()));
    t->start(0);
}

void imagewidget::stop()
{
    delete t;
    t = new QTimer;
    connect(t, SIGNAL(timeout()), this, SLOT(fadeout()));
    t->start(0);
}

void imagewidget::fade()
{
    qint64 timepassed = QDateTime::currentMSecsSinceEpoch() - animstarttime;

    if (timepassed >= animtime)
    {
        animtimer->stop();
    }
    else
    {
        opacity = (qreal(timepassed) / qreal(animtime)) / 2;
        update();
    }
}

void imagewidget::fadeblack()
{
    if (opacity < 1.0)
    {
        opacity = opacity + 0.05;
        update();
    }
    else
    {
        t->stop();
    }
}

void imagewidget::fadeout()
{
    if (opacity > 0.0)
    {
        opacity = opacity - 0.05;
        update();
    }
    else
    {
        t->stop();
        this->close();
    }
}
