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

#include "catlistwidget.h"

#include <QStyleOptionButton>

catlistwidget::catlistwidget(){
    basevlayout->setMargin(0);
    basevlayout->setSpacing(0);
    basevlayout->addStretch(5);
}

void catlistwidget::clear(){
    foreach (catlistitem *item, item_list) {
        basevlayout->removeWidget(item);
        delete item;
    }
    item_list.clear();

    foreach (QLabel *item, seperator_list) {
        basevlayout->removeWidget(item);
        delete item;
    }
    seperator_list.clear();
}

void catlistwidget::additem(QUuid id, QString text, QIcon icon){
    catlistitem *item = new catlistitem(id, text, icon);
    connect(item, SIGNAL(clicked(QUuid)), this, SLOT(handleitemclicked(QUuid)));
    connect(this, SIGNAL(currentRowChanged(QUuid)), item, SLOT(updatepressed(QUuid)));
    if (basevlayout->count() < 2)
        basevlayout->insertWidget(0, item);
    else
        basevlayout->insertWidget(basevlayout->count()-1, item);
    item_list.append(item);
}

void catlistwidget::addseperator(QString text){
    QLabel *textlabel = new QLabel(text);
    textlabel->setObjectName("CategoryDivider");
    if (basevlayout->count() < 2)
        basevlayout->insertWidget(0, textlabel);
    else
        basevlayout->insertWidget(basevlayout->count()-1, textlabel);
    seperator_list.append(textlabel);
}

void catlistwidget::handleitemclicked(QUuid id){
    emit currentRowChanged(id);
}

//listitem class~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

catlistitem::catlistitem(QUuid id, QString text, QIcon icon){
    item_id = id;
    item_text = text;
    item_icon = icon;

    setObjectName("CategoryButton");
}

void catlistitem::updatepressed(QUuid id){
    if (id != item_id){
        pressed = false;
        update();
    }
    else{
        pressed = true;
        update();
    }
}

void catlistitem::enterEvent(QEvent *){
    highlight = true;
    update();
}

void catlistitem::leaveEvent(QEvent *){
    highlight = false;
    update();
}

void catlistitem::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton){
        pressed = true;
        mousepressed = true;
        update();
    }
}

void catlistitem::mouseReleaseEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton){
        mousepressed = false;
        update();
        emit clicked(item_id);
    }
}

void catlistitem::paintEvent(QPaintEvent *){
    QStyleOptionButton option;
    option.initFrom(this);
    if (mousepressed || pressed)
        option.state |= QStyle::State_Sunken;
    else if (highlight)
        option.state |= QStyle::State_MouseOver;
    else
        option.state |= QStyle::State_Raised;

    QSize iconsize = QSize(22,22);
    if (item_text!="" && !item_icon.isNull())
        option.text = option.fontMetrics.elidedText(item_text, Qt::ElideRight, style()->subElementRect(QStyle::SE_PushButtonContents, &option, this).width() - iconsize.width()-2);
    else
        option.text = item_text;

    option.icon = item_icon;
    option.iconSize = iconsize;

    QPainter painter(this);
    style()->drawControl(QStyle::CE_PushButton, &option, &painter, this);
}
