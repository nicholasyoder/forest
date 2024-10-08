#ifndef WALLPAPERWIDGET_H
#define WALLPAPERWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QMenu>
#include <QMouseEvent>
#include <QDebug>

#include "global_settings.h"

class wallpaperwidget : public QWidget
{
    Q_OBJECT
public:
    wallpaperwidget(QImage *image, WALLPAPER_MODE imode = Fill);

public slots:
    void setwallpaper(QImage *image){ wallpaper = image; setup_wallpaper(); }
    void setimagemode(WALLPAPER_MODE imode){ imagemode = imode; setup_wallpaper(); }
    void setcontextmenu(QMenu *contextmenu){ cmenu = contextmenu;}

protected:
    void paintEvent(QPaintEvent *);
    void mouseReleaseEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *){ setup_wallpaper(); }

private slots:
    void setup_wallpaper();

private:
    QImage *wallpaper = new QImage();
    QImage *scaledwallpaper = new QImage();
    QMenu *cmenu = nullptr;
    WALLPAPER_MODE imagemode;
};

#endif // WALLPAPERWIDGET_H
