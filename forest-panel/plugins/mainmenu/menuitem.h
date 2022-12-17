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

#ifndef MENUITEM_H
#define MENUITEM_H

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QUuid>
#include <QStyleOptionButton>

#include <qt5xdg/XdgDesktopFile>
#include <qt5xdg/XdgIcon>

#include "globals.h"

class menuitem : public QFrame
{
    Q_OBJECT

public:
    menuitem(QUuid id, QIcon icon, QString text);

signals:
    void activated(QUuid id, ITEM_EVENT eventtype, QEvent *event);
    void focused(menuitem *item);

public slots:
    void setState(ITEM_STATE state);
    void setSelected(bool selected = true){itemSelected = selected; update();}
    QUuid uuid(){return itemID;}

protected:
    void paintEvent(QPaintEvent *);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    ITEM_STATE currentState;
    QUuid itemID;
    QIcon itemIcon;
    QString itemText;
    bool itemSelected = false;
    bool allowDrag = false;
    QPoint dragStartPos;
};

#endif // MENUITEM_H
