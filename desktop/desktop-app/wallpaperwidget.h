// SPDX-License-Identifier: LGPL-3.0-or-later

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
    wallpaperwidget(const QImage &image, WALLPAPER_MODE imode, QScreen *screen);

public slots:
    void setwallpaper(const QImage &image, WALLPAPER_MODE imode){ wallpaper = image; imagemode = imode; setup_wallpaper(); }
    void setcontextmenu(QMenu *contextmenu){ cmenu = contextmenu;}

protected:
    void paintEvent(QPaintEvent *);
    void mouseReleaseEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *){ if (size() != scaledwallpaper.size()) setup_wallpaper(); }

private slots:
    void setup_wallpaper();

private:
    QImage wallpaper;
    QImage scaledwallpaper;
    QMenu *cmenu = nullptr;
    WALLPAPER_MODE imagemode = Fill;
};

#endif // WALLPAPERWIDGET_H
