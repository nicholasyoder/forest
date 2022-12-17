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

#include "iconbutton.h"

iconbutton::iconbutton(QIcon icon, int w, int h, bool havetext, QString text)
{
    setFocusPolicy(Qt::StrongFocus);
    setObjectName("logout_Button");

    if (havetext == true)
    {
        QVBoxLayout *basevlayout = new QVBoxLayout;
        QHBoxLayout *icohlayout = new QHBoxLayout;
        icohlayout->addStretch(5);
        iconlabel->setPixmap(icon.pixmap(w,h));
        icohlayout->addWidget(iconlabel);
        icohlayout->addStretch(5);
        basevlayout->addLayout(icohlayout);

        QHBoxLayout *texthlayout = new QHBoxLayout;
        texthlayout->addStretch(5);
        textlabel->setText(text);
        textlabel->setForegroundRole(QPalette::Light);
        texthlayout->addWidget(textlabel);
        texthlayout->addStretch(5);
        basevlayout->addLayout(texthlayout);
        this->setLayout(basevlayout);
    }
    else
    {
        QVBoxLayout *basevlayout = new QVBoxLayout;
        iconlabel->setPixmap(icon.pixmap(w,h));
        basevlayout->addWidget(iconlabel);
        this->setLayout(basevlayout);
    }
}

void iconbutton::paintEvent(QPaintEvent *)
{
    QStyleOptionButton option;
    option.initFrom(this);
    if (pressed)
        option.state |= QStyle::State_Sunken;
    else if (focused)
        option.state |= QStyle::State_MouseOver;
    else
        option.state |= QStyle::State_Raised;

    option.state.setFlag(QStyle::State_HasFocus, false);

    QPainter painter(this);
    style()->drawControl(QStyle::CE_PushButton, &option, &painter, this);
}

void iconbutton::focusInEvent(QFocusEvent *)
{
    focused = true;
    update();
}

void iconbutton::focusOutEvent(QFocusEvent *)
{
    focused = false;
    update();
}

void iconbutton::mousePressEvent(QMouseEvent *)
{
    pressed = true;
    update();
}

void iconbutton::mouseReleaseEvent(QMouseEvent *)
{
    pressed = false;
    update();

    emit clicked();
}

void iconbutton::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        emit clicked();
    else
        event->ignore();
}
