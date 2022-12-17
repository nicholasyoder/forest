#include "wallpaperwidget.h"

wallpaperwidget::wallpaperwidget(QImage *image, ImageMode imode)
{
    Qt::WindowFlags flags;
    flags |= Qt::FramelessWindowHint;
    flags |= Qt::WindowStaysOnBottomHint;
    this->setWindowFlags(flags);
    this->setAttribute(Qt::WA_X11NetWmWindowTypeDesktop);

    wallpaper = image;
    imagemode = imode;

    setupwallpaper();
}

void wallpaperwidget::paintEvent(QPaintEvent *)
{
    QRectF target(0.0, 0.0, this->width(), this->height());
    QRectF source(0.0, 0.0, scaledwallpaper->width(), scaledwallpaper->height());
    QPainter painter(this);
    painter.drawImage(target, *scaledwallpaper, source);
}

void wallpaperwidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        if (cmenu) cmenu->exec(event->pos());
}

void wallpaperwidget::setupwallpaper()
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

void wallpaperwidget::paintFill()
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

void wallpaperwidget::paintFit()
{
    QImage scaledw = wallpaper->scaled(this->width(), this->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    int x = this->width()/2 - scaledw.width()/2;
    int y = this->height()/2 - scaledw.height()/2;

    QRectF target(x, y, scaledw.width(), scaledw.height());
    QRectF source(0, 0, scaledw.width(), scaledw.height());
    QPainter painter(scaledwallpaper);
    painter.drawImage(target, scaledw, source);
}

void wallpaperwidget::paintStretch()
{
    QImage scaledw = wallpaper->scaled(this->width(), this->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QRectF target(0.0, 0.0, this->width(), this->height());
    QRectF source(0.0, 0.0, scaledw.width(), scaledw.height());
    QPainter painter(scaledwallpaper);
    painter.drawImage(target, scaledw, source);
}

void wallpaperwidget::paintTile()
{

}

void wallpaperwidget::paintCenter()
{
    int x = this->width()/2 - wallpaper->width()/2;
    int y = this->height()/2 - wallpaper->height()/2;

    QRectF target(x, y, wallpaper->width(), wallpaper->height());
    QRectF source(0, 0, wallpaper->width(), wallpaper->height());
    QPainter painter(scaledwallpaper);
    painter.drawImage(target, *wallpaper, source);
}
