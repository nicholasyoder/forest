// SPDX-License-Identifier: LGPL-3.0-or-later

#include "deskbutton.h"

deskbutton::deskbutton(int desknumber)
{
    desknum = desknumber;
    this->setFixedWidth(23);

    //connect(this, &deskbutton::leftclicked, this, &deskbutton::handleclick);
    //setupbt();
    //setbuttondata("", "draw-rectangle");
}

void deskbutton::setactive(int num)
{
    if (num == desknum)
    {
        active = true;
        this->update();
    }
    else
    {
        active = false;
        //activeicon = X11UTILLS::getWindowIcon(X11UTILLS::getActiveWindow());
        this->update();
    }

}

void deskbutton::paintEvent(QPaintEvent *)
{
    QSize outlinesize(this->width() - 4, this->height() - 4);
    int boxsize = (outlinesize.width() /2) - 2;

    QPainter p(this);
    p.setPen(Qt::lightGray);
    QRect outline(2,2, outlinesize.width(), outlinesize.height());

    if (active){
        p.setPen(QColor(100,170,100));
        p.setBrush(QColor(150,150,150, 80));
    }
    p.drawRect(outline);
    p.setBrush(Qt::gray);

    int boxX = width()/2 - boxsize/2;
    int boxY = height()/2 - boxsize/2;

    if (numDeskWindows == 1){
        p.drawRect(boxX, boxY, boxsize, boxsize);
    }
    else if (numDeskWindows == 2){
        p.drawRect(boxX-1, boxY-1, boxsize, boxsize);
        p.drawRect(boxX+1, boxY+1, boxsize, boxsize);
    }
    else if (numDeskWindows > 2){
        p.drawRect(boxX-2, boxY-2, boxsize, boxsize);
        p.drawRect(boxX, boxY, boxsize, boxsize);
        p.drawRect(boxX+2, boxY+2, boxsize, boxsize);
    }
}

void deskbutton::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        emit clicked(desknum);

    event->ignore();
}
