// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WALLPAPERWIDGET_H
#define WALLPAPERWIDGET_H

#include <QMenu>
#include <QMouseEvent>

#include "wallpaper.h"

class QScreen;

class wallpaperwidget : public wallpaper::WallpaperView
{
    Q_OBJECT
public:
    wallpaperwidget(wallpaper::WallpaperSource *source, QScreen *screen);

public slots:
    void setcontextmenu(QMenu *contextmenu){ cmenu = contextmenu; }

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QMenu *cmenu = nullptr;
};

#endif // WALLPAPERWIDGET_H
