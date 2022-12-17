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

#ifndef ICONBUTTON_H
#define ICONBUTTON_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPainter>
#include <QDebug>
#include <QKeyEvent>
#include <QStyleOptionButton>

class iconbutton : public QFrame
{
    Q_OBJECT
public:
    iconbutton(QIcon icon,int w, int h, bool havetext = false, QString text = "");

signals:
    void clicked();

public slots:

private:
    QLabel *iconlabel = new QLabel;
    QLabel *textlabel = new QLabel;

    bool focused = false;
    bool pressed = false;

private slots:
    void paintEvent(QPaintEvent *);
    void enterEvent(QEvent *){this->setFocus();}
    void focusInEvent(QFocusEvent *);
    void leaveEvent(QEvent *){this->clearFocus();}
    void focusOutEvent(QFocusEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *event);
};

#endif // ICONBUTTON_H
