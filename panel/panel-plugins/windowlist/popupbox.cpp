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

#include "popupbox.h"

popupbox::popupbox(unsigned long window)
{
    Qt::WindowFlags flags;
    flags |= Qt::X11BypassWindowManagerHint;
    flags |= Qt::Popup;
    flags |= Qt::WindowStaysOnTopHint;
    flags |= Qt::CustomizeWindowHint;
    this->setWindowFlags(flags);

    QVBoxLayout *layout = new QVBoxLayout;
    QIcon ico("");

    button *raisebt = new button(window,ico, "onepart", "Raise");
    raisebt->setFixedHeight(26);
    //connect(raisebt, &button::leftclicked, this, &popupbox::raisewin);
    connect(raisebt, SIGNAL(leftclicked(unsigned long)), this, SIGNAL(raisewin(unsigned long)));
    //connect(raisebt, &button::leftclicked, this, &popupbox::closepop);
    connect(raisebt, SIGNAL(leftclicked(unsigned long)), this, SLOT(closepop(unsigned long)));

    button *maxbt = new button(window,ico, "onepart", "Maximize");
    maxbt->setFixedHeight(26);
    //connect(maxbt, &button::leftclicked, this, &popupbox::maxwin);
    connect(maxbt, SIGNAL(leftclicked(unsigned long)), this, SIGNAL(maxwin(unsigned long)));
    //connect(maxbt, &button::leftclicked, this, &popupbox::closepop);
    connect(maxbt, SIGNAL(leftclicked(unsigned long)), this, SLOT(closepop(unsigned long)));

    button *minbt = new button(window,ico, "onepart", "Minimize");
    minbt->setFixedHeight(26);
    //connect(minbt, &button::leftclicked, this, &popupbox::minwin);
    connect(minbt, SIGNAL(leftclicked(unsigned long)), this, SIGNAL(minwin(unsigned long)));
    //connect(minbt, &button::leftclicked, this, &popupbox::closepop);
    connect(minbt, SIGNAL(leftclicked(unsigned long)), this, SLOT(closepop(unsigned long)));

    button *closebt = new button(window,ico, "onepart", "Close Window");
    closebt->setFixedHeight(26);
    //connect(closebt, &button::leftclicked, this, &popupbox::closewin);
    connect(closebt, SIGNAL(leftclicked(unsigned long)), this, SIGNAL(closewin(unsigned long)));
    //connect(closebt, &button::leftclicked, this, &popupbox::closepop);
    connect(closebt, SIGNAL(leftclicked(unsigned long)), this, SLOT(closepop(unsigned long)));

    layout->addWidget(raisebt);
    layout->addWidget(maxbt);
    layout->addWidget(minbt);
    layout->addWidget(closebt);

    layout->setMargin(0);
    layout->setSpacing(0);

    this->setLayout(layout);


    this->setFixedHeight(104);
}

popupbox::~popupbox()
{

}

void popupbox::paintEvent(QPaintEvent *event)
{
    QImage image(1,1,QImage::Format_RGB32);
    QColor c(25,25,25,255);
    image.fill(c);
    QRectF target(0.0, 0.0, this->width(), this->height());
    QRectF source(0.0, 0.0, image.width(), image.height());
    QPainter painter(this);
    painter.drawImage(target, image, source);
}

void popupbox::mousePressEvent(QMouseEvent *event)
{
    this->close();
}

void popupbox::leaveEvent(QEvent *event)
{
    //this->close();
}

void popupbox::closepop(unsigned long window)
{
    QTimer *t = new QTimer;
    connect(t, SIGNAL(timeout()), this, SLOT(close()));
    t->setSingleShot(true);
    t->start(200);
}

void popupbox::showanim()
{

    animtimer = new QTimer;
    animfactor = 0;
    connect(animtimer, SIGNAL(timeout()), this, SLOT(incrementanim()));

    this->show();

    animtimer->start(5);
}

void popupbox::incrementanim()
{
    if (animfactor < 20)
    {
        if (animfactor == 0)
        {
            this->scroll(0, 19);
        }
        else
        {
            this->scroll(0, -1);
        }
    }
    else
    {
        animtimer->stop();
    }

    animfactor++;
}
