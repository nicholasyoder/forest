// SPDX-License-Identifier: LGPL-3.0-or-later

#include "fadewidget.h"
#include <QScreen>

fadewidget::fadewidget(QScreen *screen){
    setWindowOpacity(0.0);
    setWindowFlags(Qt::X11BypassWindowManagerHint);
    setGeometry(screen->geometry());
    wscreen = screen;
    getimage();
}

void fadewidget::paintEvent(QPaintEvent *){
    int w = width(); int h = height();

    QRectF target1(0.0, 0.0, w, h);
    QRectF source1(0.0, 0.0, backimage->width(), backimage->height());
    QPainter painter1(this);
    painter1.drawImage(target1, *backimage, source1);

    QRectF target(0.0, 0.0, w, h);
    QRectF source(0.0, 0.0, wallpaper->width(), wallpaper->height());
    QPainter painter(this);
    painter.setOpacity(opacity);
    painter.drawImage(target, *wallpaper, source);
}

void fadewidget::getimage(){
    //grab screenshot - maybe this can get deleted after i write a session manager
    QRect screenRect = wscreen->geometry();
    *backimage = qApp->primaryScreen()->grabWindow(0, screenRect.x(), screenRect.y(), screenRect.width(), screenRect.height()).toImage();

    QSettings settings("Forest","Forest");
    imagemode = WALLPAPER_MODE(settings.value("desktop/imagemode", Fill).toInt());
    QString wallpaper_file = settings.value("desktop/wallpaper", "").toString();
    wallpaper = miscutills::get_wallpaper_scaled(wallpaper_file, imagemode, this->size());

    setWindowOpacity(1.0);
}

void fadewidget::start(){
    connect(t, SIGNAL(timeout()), this, SLOT(fadein()));
    t->start(40);
}

void fadewidget::fadein(){
    if (opacity < 1.0){
        opacity += 0.05;
        update();
    }
    else{
        t->stop();
        delete backimage;
        delete wallpaper;
        delete this;
    }
}
