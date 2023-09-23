#include "wallpaperwidget.h"

wallpaperwidget::wallpaperwidget(QImage *image, ImageMode imode){
    Qt::WindowFlags flags;
    flags |= Qt::FramelessWindowHint;
    flags |= Qt::WindowStaysOnBottomHint;
    this->setWindowFlags(flags);
    this->setAttribute(Qt::WA_X11NetWmWindowTypeDesktop);

    wallpaper = image;
    imagemode = imode;

    setup_wallpaper();
}

void wallpaperwidget::paintEvent(QPaintEvent *){
    QRectF target(0.0, 0.0, width(), height());
    QRectF source(0.0, 0.0, scaledwallpaper->width(), scaledwallpaper->height());
    QPainter painter(this);
    painter.drawImage(target, *scaledwallpaper, source);
}

void wallpaperwidget::mouseReleaseEvent(QMouseEvent *event){
    if (event->button() == Qt::RightButton)
        if (cmenu) cmenu->exec(event->pos());
}

void wallpaperwidget::setup_wallpaper(){
    scaledwallpaper = new QImage(width(), height(), wallpaper->format());
    scaledwallpaper->fill(Qt::black);

    switch (imagemode){
    case Fill: do_fill(); break;
    case Fit: do_fit(); break;
    case Stretch: do_stretch(); break;
    case Tile: do_tile(); break;
    case Center: do_center(); break;
    }

    update();
}

void wallpaperwidget::do_fill(){
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

void wallpaperwidget::do_fit(){
    QImage scaledw = wallpaper->scaled(this->width(), this->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    int x = this->width()/2 - scaledw.width()/2;
    int y = this->height()/2 - scaledw.height()/2;

    QRectF target(x, y, scaledw.width(), scaledw.height());
    QRectF source(0, 0, scaledw.width(), scaledw.height());
    QPainter painter(scaledwallpaper);
    painter.drawImage(target, scaledw, source);
}

void wallpaperwidget::do_stretch(){
    QImage scaledw = wallpaper->scaled(this->width(), this->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QRectF target(0.0, 0.0, this->width(), this->height());
    QRectF source(0.0, 0.0, scaledw.width(), scaledw.height());
    QPainter painter(scaledwallpaper);
    painter.drawImage(target, scaledw, source);
}

void wallpaperwidget::do_tile(){
    // unimplemented
}

void wallpaperwidget::do_center(){
    int x = this->width()/2 - wallpaper->width()/2;
    int y = this->height()/2 - wallpaper->height()/2;

    QRectF target(x, y, wallpaper->width(), wallpaper->height());
    QRectF source(0, 0, wallpaper->width(), wallpaper->height());
    QPainter painter(scaledwallpaper);
    painter.drawImage(target, *wallpaper, source);
}
