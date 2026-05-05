// SPDX-License-Identifier: LGPL-3.0-or-later

#include "menuitem.h"

#include <QDebug>
#include "miscutills/miscutills.h"

menuitem::menuitem(QUuid id, QIcon icon, QString text){
    itemID = id;
    itemIcon = icon;
    itemText = text;
    itemIconSize = miscutills::get_iconsize_stylesheet("#popupMenuItem", qApp->styleSheet());
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
    option.iconSize = itemIconSize;

    QPainter painter(this);
    style()->drawControl(QStyle::CE_PushButton, &option, &painter, this);
}

void menuitem::enterEvent(QEnterEvent *event){
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
