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

#include "menuitem.h"

#include <QDebug>
#include "futils/futils.h"

menuitem::menuitem(QUuid id, QIcon icon, QString text){
    itemID = id;
    itemIcon = icon;
    itemText = text;

    setContextMenuPolicy(Qt::CustomContextMenu);
    setObjectName("popupMenuItem");
}

void menuitem::setState(ITEM_STATE state){
    currentState = state;
    update();
    if (currentState == FOCUS)
        emit focused(this);
}

void menuitem::paintEvent(QPaintEvent *){
    QStyleOptionButton option;
    option.initFrom(this);

    if(itemSelected){
        option.state |= QStyle::State_Sunken;
    }
    else {
        switch (currentState) {
        case FOCUS: option.state |= QStyle::State_MouseOver; break;
        case ACTIVE: option.state |= QStyle::State_Sunken; break;
        default: option.state |= QStyle::State_Raised; break;
        }
    }

    option.state.setFlag(QStyle::State_HasFocus, false);

    option.icon = itemIcon;
    option.text = itemText;
    option.iconSize = futils::get_iconsize_stylesheet("#popupMenuItem", qApp->styleSheet());

    QPainter painter(this);
    style()->drawControl(QStyle::CE_PushButton, &option, &painter, this);
}

void menuitem::enterEvent(QEvent *event){
    currentState = FOCUS;
    emit activated(itemID, ENTER, event);
}

void menuitem::leaveEvent(QEvent *event){
    currentState = NORMAL;
    emit activated(itemID, LEAVE, event);
}

void menuitem::mousePressEvent(QMouseEvent *event){
    currentState = ACTIVE;
    emit activated(itemID, PRESS, event);
    if (event->button() == Qt::LeftButton){
        dragStartPos = event->pos();
        allowDrag = true;
    }
}

void menuitem::mouseReleaseEvent(QMouseEvent *event){
    //hack to keep drag and drop release events from running app
    if (currentState == ACTIVE){
        currentState = FOCUS;
        emit activated(itemID, RELEASE, event);
        allowDrag = false;
    }
}

void menuitem::mouseMoveEvent(QMouseEvent *event){
    if (allowDrag && (event->pos() - dragStartPos).manhattanLength() > QApplication::startDragDistance()){
        currentState = NORMAL;
        emit activated(itemID, DRAG, event);
    }
}
