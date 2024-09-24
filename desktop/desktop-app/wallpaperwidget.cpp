#include "wallpaperwidget.h"

wallpaperwidget::wallpaperwidget(QImage *image, WALLPAPER_MODE imode){
    Qt::WindowFlags flags;
    flags |= Qt::FramelessWindowHint;
    flags |= Qt::WindowStaysOnBottomHint;
    setWindowFlags(flags);
    setAttribute(Qt::WA_X11NetWmWindowTypeDesktop);

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
    scaledwallpaper = miscutills::get_wallpaper_scaled(wallpaper, imagemode, size());
    update();
}

