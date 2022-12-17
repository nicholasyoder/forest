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

#ifndef ANIMWIDGET_H
#define ANIMWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QApplication>
#include <QPainter>
#include <QDesktopWidget>
#include <QSettings>
#include <QDebug>
#include <QScreen>

class animwidget : public QWidget
{
    Q_OBJECT
public:
    animwidget();

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
    void paintEvent(QPaintEvent *event);

private slots:
    void fadein();
    void fadeblack();
    void fadeout();
};

#endif // ANIMWIDGET_H
