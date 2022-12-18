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

#include "button.h"

button::button(xcb_window_t window, QIcon icon, QString type, QString text){
    bttype = type;
    bttext = text;
    btwindow = window;

    if (bttype == "icononly"){
        setupIconButton(icon, 16);
    }
    else if (bttype == "tab"){//take this out or make it work again
        setupIconAndTextButton(bttext, icon, 16);
        /*if (hlayout){
            closebutton *cbt = new closebutton;
            cbt->setFixedSize(18,18);
            hlayout->addWidget(cbt);
            connect(cbt, &closebutton::clicked, this, &button::closewindow);
        }*/
    }
    else{
        setupIconAndTextButton(bttext, icon, 16);
    }

    pmenu = new popupmenu(this, CenteredOnWidget);

    pmenuitem *item = new pmenuitem("Raise", QIcon::fromTheme("arrow-up"));
    connect(item, &pmenuitem::clicked, this, &button::raisewindow);
    pmenu->additem(item);
    pmenuitem *item2 = new pmenuitem("Maximize", QIcon::fromTheme("arrow-up-double"));
    connect(item2, &pmenuitem::clicked, this, &button::maximizewindow);
    pmenu->additem(item2);
    pmenuitem *item3 = new pmenuitem("Demaximize", QIcon::fromTheme("arrow-down"));
    connect(item3, &pmenuitem::clicked, this, &button::demaximizewindow);
    pmenu->additem(item3);
    pmenuitem *item4 = new pmenuitem("Minimize", QIcon::fromTheme("arrow-down-double"));
    connect(item4, &pmenuitem::clicked, this, &button::minimizewindow);
    pmenu->additem(item4);
    pmenuitem *item5 = new pmenuitem("Close", QIcon::fromTheme("application-exit"));
    connect(item5, &pmenuitem::clicked, this, &button::closewindow);
    pmenu->additem(item5);

    connect(this, SIGNAL(clicked(QMouseEvent*)), this, SLOT(slotclicked(QMouseEvent*)));

    /*QVBoxLayout *popupvlayout = new QVBoxLayout;
    popupvlayout->setMargin(0);
    popupvlayout->setSpacing(3);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setMargin(0);
    //wintitlelabel = new QLabel;
    //hlayout->addWidget(wintitlelabel);
    hlayout->addStretch(1);
    closebutton *cbt = new closebutton;
    cbt->setFixedSize(18,18);
    hlayout->addWidget(cbt);
    popupvlayout->addLayout(hlayout);

    scrshotlabel = new QLabel;
    popupvlayout->addWidget(scrshotlabel);
    pbox = new popup(popupvlayout, this, CenteredOnWidget);
    connect(cbt, &closebutton::clicked, pbox, &popup::closepopup);
    connect(cbt, &closebutton::clicked, this, &button::closewindow);*/
    connect(this, &button::enterevent, this, &button::handleEnterEvent);
    connect(this, &button::leaveevent, this, &button::handleLeaveEvent);
}

button::~button(){
    pmenu->deleteLater();
}

void button::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton){
        mousepressed = true;
        startdragpos = event->pos();
    }
}

void button::mouseReleaseEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton){
        if (mousepressed){
            emit clicked(event);
            mousepressed = false;
        }
    }
    else{
        emit clicked(event);
    }

}

void button::mouseMoveEvent(QMouseEvent *event){
    if (mousepressed){
        if ((event->pos() - startdragpos).manhattanLength() > 20){
            demaximizewindow();
            mousepressed = false;
        }
    }
}

void button::updatedata(){ //get rid of this somehow...
    if (this->property("buttontype") != "Icon"){
        QString newtext = Xcbutills::getWindowTitle(btwindow);
        if (bttext != newtext){
            bttext = newtext;
            setText(bttext);
            setIcon(Xcbutills::getWindowIcon(btwindow));
        }
    }
}

void button::slotclicked(QMouseEvent *event){
    if (event->button() == Qt::LeftButton){
        setDown(true);
        raisewindow();
    }
    else if (event->button() == Qt::RightButton){
        QSize sizeHint = pmenu->popupw->sizeHint();
        if (width() > sizeHint.width())
            pmenu->popupw->setFixedSize(width(), sizeHint.height());
        else
            pmenu->popupw->setFixedSize(sizeHint.width(), sizeHint.height());

        pmenu->show();
    }
}

void button::sethighlight(xcb_window_t win){
    if (win == btwindow) setDown(true);
    else setDown(false);
}

void button::raisewindow(){
    Xcbutills::raiseWindow(btwindow);
}

void button::maximizewindow(){
    Xcbutills::maximizeWindow(btwindow);
}

void button::minimizewindow(){
    Xcbutills::minimizeWindow(btwindow);
}

void button::closewindow(){
    Xcbutills::closeWindow(btwindow);
}

void button::demaximizewindow(){
    Xcbutills::demaximizeWindow(btwindow);
    //QTimer::singleShot(100, this, SLOT(resizeifneeded()));
}

void button::resizeifneeded(){
    /*QRect wingeo = Xcbutills::getwindowGeometry(btwindow);
    QRect parentgeo = this->parentWidget()->parentWidget()->geometry();
    if (wingeo.intersects(parentgeo))
    {
        Xcbutills::moveWindow(btwindow, 50,50);
        QRect desktopgeo = qApp->desktop()->availableGeometry();
        Xcbutills::resizeWindow(btwindow, desktopgeo.width() - 100, desktopgeo.height() - 100);
    }*/
}

void button::handleEnterEvent(){
    emit mouseEnter(this);
    /*if (openptimer)
        delete openptimer;

    openptimer = new QTimer;
    openptimer->setSingleShot(true);
    connect(openptimer, &QTimer::timeout, this, &button::showpopup);
    openptimer->start(500);*/
}

void button::handleLeaveEvent(){
    emit mouseLeave(this);
    /*if (openptimer)
    {
        openptimer->stop();
        delete openptimer;
        openptimer = nullptr;
    }*/
}

/*void button::showpopup()
{
    QPixmap pix = QPixmap::fromImage(Xcbutills::getWindowImage(btwindow));
    scrshotlabel->setPixmap(pix.scaled(200,200, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    //wintitlelabel->setText(Xcbutills::getWindowTitle(btwindow));
    pbox->showpopup();

    closeptimer = new QTimer;
    connect(closeptimer, &QTimer::timeout, this, &button::closepopup);
    closeptimer->start(200);
}

void button::closepopup()
{
    QRect r(0,0, width(), height());

    if (!(r.contains(this->mapFromGlobal(QCursor::pos())) || pbox->geometry().contains(QCursor::pos())))
    {
        pbox->closepopup();

        if (closeptimer)
        {
            closeptimer->stop();
            delete closeptimer;
            closeptimer = nullptr;
        }
    }
}*/
