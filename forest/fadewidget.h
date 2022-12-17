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

#ifndef FADEWIDGET_H
#define FADEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QSettings>
//#include <QScreen>

enum ImageMode { Fill, Fit, Stretch, Tile, Center};

class fadewidget : public QWidget
{
    Q_OBJECT

public:
    fadewidget(QScreen *screen);

    qreal opacity = 0.0;
    QImage *backimage = new QImage();
    QImage *wallpaper = new QImage();
    QTimer *t = new QTimer;

public slots:
    void getimage();
    void start();
    void blackout();
    void stop();

protected:
    void paintEvent(QPaintEvent *);

private slots:
    void fadein();
    void fadeblack();
    void fadeout();

    void paintFill();
    void paintFit();
    void paintStretch();
    void paintTile();
    void paintCenter();

    void setupwallpaper();

private:
    //QImage *wallpaper = new QImage();
    QImage *scaledwallpaper = new QImage();
    ImageMode imagemode;
    QScreen *wscreen;
};

#endif // FADEWIDGET_H
