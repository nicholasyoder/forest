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

fadewidget::fadewidget(QScreen *screen){
    setWindowOpacity(0.0);
    setWindowFlags(Qt::X11BypassWindowManagerHint);
    wscreen = screen;
}

void fadewidget::paintEvent(QPaintEvent *){
    QRectF target1(0.0, 0.0, this->width(), this->height());
    QRectF source1(0.0, 0.0, backimage->width(), backimage->height());
    QPainter painter1(this);
    painter1.drawImage(target1, *backimage, source1);

    QRectF target(0.0, 0.0, this->width(), this->height());
    QRectF source(0.0, 0.0, wallpaper->width(), wallpaper->height());
    QPainter painter(this);
    painter.setOpacity(opacity);
    painter.drawImage(target, *wallpaper, source);
}

void fadewidget::getimage(){
    //grab screenshot - maybe this can get deleted after i write a session manager
    *backimage = wscreen->grabWindow(QApplication::desktop()->winId(), wscreen->geometry().x(), wscreen->geometry().y(), width(), height()).toImage();

    QSettings settings("Forest","Forest");
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

    QString wallpaper_file = settings.value("desktop/wallpaper", "").toString();
    if(!wallpaper_file.isEmpty())
        wallpaper = miscutills::get_wallpaper_scaled(wallpaper_file, imagemode, this->size());

    setWindowOpacity(1.0);
}

void fadewidget::start(){
    connect(t, SIGNAL(timeout()), this, SLOT(fadein()));
    t->start(40);
}

void fadewidget::blackout(){
    delete t;
    t = new QTimer;
    connect(t, SIGNAL(timeout()), this, SLOT(fadeblack()));
    t->start(40);
}

void fadewidget::stop(){
    delete t;
    t = new QTimer;
    connect(t, SIGNAL(timeout()), this, SLOT(fadeout()));
    t->start(40);
}

void fadewidget::fadein(){
    if (opacity < 1.0){
        opacity = opacity + 0.05;
        update();
    }
    else{
        t->stop();
        delete backimage;
        delete wallpaper;
        delete this;
    }
}

void fadewidget::fadeblack(){
    if (opacity < 1.0){
        opacity = opacity + 0.05;
        update();
    }
    else{
        t->stop();
    }
}

void fadewidget::fadeout(){
    if (opacity > 0.0){
        opacity = opacity - 0.05;
        update();
    }
    else{
        t->stop();
        this->close();
    }
}
