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

#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QProcess>
#include <QPainter>
#include <QDir>
#include <QDateTime>

class imagewidget : public QWidget
{
    Q_OBJECT

public:
    imagewidget();

    qreal opacity = 0.0;
    QImage *backimage = new QImage();
    QTimer *t = new QTimer;

public slots:
    void start();
    void blackout();
    void stop();

private slots:
    void paintEvent(QPaintEvent *);

    void fade();
    void fadeblack();
    void fadeout();

private:
    qint64 animstarttime = 0;
    qint64 animtime = 0;
    QTimer *animtimer = new QTimer;

};

#endif // IMAGEWIDGET_H
