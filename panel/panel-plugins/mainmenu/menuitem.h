// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MENUITEM_H
#define MENUITEM_H

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QUuid>
#include <QStyleOptionButton>

#include <qt6xdg/XdgDesktopFile>
#include <qt6xdg/XdgIcon>

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
    void enterEvent(QEnterEvent *event);
    void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    ITEM_STATE currentState;
    QUuid itemID;
    QIcon itemIcon;
    QString itemText;
    QSize itemIconSize;
    bool itemSelected = false;
    bool allowDrag = false;
    QPoint dragStartPos;
};

#endif // MENUITEM_H
