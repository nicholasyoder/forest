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

#include "fadewidget.h"
#include <QScreen>

fadewidget::fadewidget(QScreen *screen)
{
    this->setWindowOpacity(0.0);
    this->setWindowFlags(Qt::X11BypassWindowManagerHint);

    wscreen = screen;
}

void fadewidget::paintEvent(QPaintEvent *)
{
    QRectF target1(0.0, 0.0, this->width(), this->height());
    QRectF source1(0.0, 0.0, backimage->width(), backimage->height());
    QPainter painter1(this);
    painter1.drawImage(target1, *backimage, source1);

    QRectF target(0.0, 0.0, this->width(), this->height());
    QRectF source(0.0, 0.0, scaledwallpaper->width(), scaledwallpaper->height());
    QPainter painter(this);
    painter.setOpacity(opacity);
    painter.drawImage(target, *scaledwallpaper, source);
}

//grab screenshot - maybe this can get deleted after i write a session manager
void fadewidget::getimage()
{
    QSettings settings("Forest","Forest");
    *wallpaper = QImage(settings.value("desktop/wallpaper", "").toString());
    *backimage = wscreen->grabWindow(QApplication::desktop()->winId(), wscreen->geometry().x(), wscreen->geometry().y(), width(), height()).toImage();

    QString imode = settings.value("desktop/imagemode", "Fill").toString();
    if (imode == "Fill")
        imagemode = Fill;
    else if (imode == "Fit")
        imagemode = Fit;
    else if (imode == "Stretch")
        imagemode = Stretch;
    else if (imode == "Tile")
        imagemode = Tile;
    else if (imode == "Center")
        imagemode = Center;

    setupwallpaper();
    this->setWindowOpacity(1.0);
}

void fadewidget::setupwallpaper()
{
    scaledwallpaper = new QImage(width(), height(), wallpaper->format());
    scaledwallpaper->fill(Qt::black);

    switch (imagemode)
    {
    case Fill:
        paintFill();
        break;
    case Fit:
        paintFit();
        break;
    case Stretch:
        paintStretch();
        break;
    case Tile:
        paintTile();
        break;
    case Center:
        paintCenter();
        break;
    }

    update();
}

void fadewidget::paintFill()
{
    QImage scaledw = wallpaper->scaled(this->width(), this->height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    int x = 0, y = 0;
    if (scaledw.width() > this->width())
        x =  (scaledw.width() - this->width()) / 2;
    else if (scaledw.height() > this->height())
        y =  (scaledw.height() - this->height()) / 2;

    QRectF target(0, 0, this->width(), this->height());
    QRectF source(x, y, this->width(), this->height());
    QPainter painter(scaledwallpaper);
    painter.drawImage(target, scaledw, source);
}

void fadewidget::paintFit()
{
    QImage scaledw = wallpaper->scaled(this->width(), this->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    int x = this->width()/2 - scaledw.width()/2;
    int y = this->height()/2 - scaledw.height()/2;

    QRectF target(x, y, scaledw.width(), scaledw.height());
    QRectF source(0, 0, scaledw.width(), scaledw.height());
    QPainter painter(scaledwallpaper);
    painter.drawImage(target, scaledw, source);
}

void fadewidget::paintStretch()
{
    QImage scaledw = wallpaper->scaled(this->width(), this->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QRectF target(0.0, 0.0, this->width(), this->height());
    QRectF source(0.0, 0.0, scaledw.width(), scaledw.height());
    QPainter painter(scaledwallpaper);
    painter.drawImage(target, scaledw, source);
}

void fadewidget::paintTile()
{

}

void fadewidget::paintCenter()
{
    int x = this->width()/2 - wallpaper->width()/2;
    int y = this->height()/2 - wallpaper->height()/2;

    QRectF target(x, y, wallpaper->width(), wallpaper->height());
    QRectF source(0, 0, wallpaper->width(), wallpaper->height());
    QPainter painter(scaledwallpaper);
    painter.drawImage(target, *wallpaper, source);
}

void fadewidget::start()
{
    connect(t, SIGNAL(timeout()), this, SLOT(fadein()));
    t->start(40);
}

void fadewidget::blackout()
{
    delete t;
    t = new QTimer;
    connect(t, SIGNAL(timeout()), this, SLOT(fadeblack()));
    t->start(40);
}

void fadewidget::stop()
{
    delete t;
    t = new QTimer;
    connect(t, SIGNAL(timeout()), this, SLOT(fadeout()));
    t->start(40);
}

void fadewidget::fadein()
{
    if (opacity < 1.0)
    {
        opacity = opacity + 0.05;
        update();
    }
    else
    {
        t->stop();
        delete backimage;
        delete wallpaper;
        delete this;
    }
}

void fadewidget::fadeblack()
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

void fadewidget::fadeout()
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
