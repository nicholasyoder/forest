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

#include "listwidget.h"

listwidget::listwidget()
{
    basevlayout->setMargin(0);
    basevlayout->setSpacing(0);
    basevlayout->addStretch(5);

    setObjectName("settingsSideBar");
}

void listwidget::additem(QString text, QIcon icon)
{
    listitem *item = new listitem(text, icon);
    connect(item, SIGNAL(clicked(QString)), this, SLOT(handleitemclicked(QString)));
    connect(this, SIGNAL(updatepressed(QString)), item, SLOT(updatepressed(QString)));
    itemlist.append(text);
    //basevlayout->addWidget(item);
    if (basevlayout->count() < 2)
        basevlayout->insertWidget(0, item);
    else
        basevlayout->insertWidget(basevlayout->count()-1, item);
}

void listwidget::addseperator(QString text)
{
    QLabel *textlabel = new QLabel(text);
    QFont f;
    f.setBold(true);
    textlabel->setFont(f);
    //basevlayout->addWidget(textlabel);
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setMargin(2);
    hlayout->addSpacing(2);
    hlayout->addWidget(textlabel);

    if (basevlayout->count() < 2)
        basevlayout->insertLayout(0, hlayout);
    else
        basevlayout->insertLayout(basevlayout->count()-1, hlayout);
}

void listwidget::handleitemclicked(QString itemtext)
{
    emit currentRowChanged(itemlist.indexOf(itemtext));
    emit updatepressed(itemtext);
}

//listitem class~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

listitem::listitem(QString text, QIcon icon)
{
    itemtext = text;

    QHBoxLayout *hlayout = new QHBoxLayout;
    QLabel *iconlabel = new QLabel;
    QLabel *textlabel = new QLabel(text);
    iconlabel->setPixmap(icon.pixmap(22,22));
    hlayout->addWidget(iconlabel);
    hlayout->addWidget(textlabel);
    hlayout->addStretch(5);
    this->setLayout(hlayout);
}

void listitem::updatepressed(QString currentitemtext)
{
    if (currentitemtext != itemtext)
    {
        pressed = false;
        update();
    }
    else
    {
        pressed = true;
        update();
    }
}

void listitem::enterEvent(QEvent *)
{
    highlight = true;
    update();
}

void listitem::leaveEvent(QEvent *)
{
    highlight = false;
    update();
}

void listitem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        pressed = true;
        mousepressed = true;
        update();
    }
}

void listitem::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        mousepressed = false;
        update();
        emit clicked(itemtext);
    }
}

void listitem::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if (mousepressed)
        painter.fillRect(0,0, this->width(), this->height(), QColor::fromRgb(220,220,220));
    else if (highlight)
        painter.fillRect(0,0, this->width(), this->height(), QColor::fromRgb(225,225,225));
    else if (pressed)
        painter.fillRect(0,0, this->width(), this->height(), QColor::fromRgb(220,220,220));
}
