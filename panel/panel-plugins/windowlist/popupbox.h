/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)GPL3+
 *
 * Copyright: 2020 Nicholas Yoder
 * Authors:
 *   Nicholas Yoder <nobody@noemail.com>
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

#ifndef POPUPBOX_H
#define POPUPBOX_H

#include <QWidget>
#include <QTimer>
#include "button.h"

class popupbox : public QWidget
{
    Q_OBJECT

public:
    popupbox(unsigned long window);
    ~popupbox();

    void showanim();

signals:
    //void clicked();
    void raisewin(unsigned long window);
    void maxwin(unsigned long window);
    void minwin(unsigned long window);
    void closewin(unsigned long window);

private slots:
    void closepop(unsigned long window);
    void incrementanim();

    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);

private:

    QTimer *animtimer = new QTimer;
    int animfactor = 0;

};

#endif // POPUPBOX_H
