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

#include "windowlist.h"

windowlist::windowlist()
{
}

windowlist::~windowlist()
{
}

void windowlist::setupPlug(QBoxLayout *layout, QList<pmenuitem *> itemlist)
{
    layout->addWidget(this);

    mainlayout->setDirection(layout->direction());
    mainlayout->setMargin(0);
    mainlayout->setSpacing(0);
    this->setLayout(mainlayout);

    pmenu = new popupmenu(this, CenteredOnMouse);
    foreach (pmenuitem *item, itemlist)
        pmenu->additem(item);

    pmenu->addseperator();
    pmenuitem *item = new pmenuitem("Windowlist Settings", QIcon::fromTheme("configure"));
    connect(item, &pmenuitem::clicked, this, &windowlist::showsettingswidget);
    pmenu->additem(item);

    ipopup = new imagepopup(this);

    loadsettings();

    currentdesk = Xcbutills::getCurrentDesktop();
    updatelist();

    //this updates the text on the button (for like when you change directories in a file manager & the window title changes too.)
    //there should be a better way to do this instead of running a timer all the time. eats cpu time
    QTimer *t = new QTimer;
    connect(t, SIGNAL(timeout()), this, SIGNAL(updatebuttondata()));
    t->start(800);
}

QHash<QString, QString> windowlist::getpluginfo()
{
    QHash<QString, QString> info;
    info["name"] = "Window List";
    info["needsXcbEvents"] = "true";
    info["stretch"] = "true";
    return info;
}

void windowlist::updatelist()
{
    QList<xcb_window_t> newwindows = Xcbutills::getClientList();
    if (newwindows.length() == 0)
        mainlayout->addWidget(stretchwidget, 5);

    //if newwindow list != old window list or if the desktop has changed
    if (newwindows != oldwindows || currentdesk != olddesk)
    {
        olddesk = currentdesk;
        //check if window is on the current desktop
        int i = 0;
        while (i < newwindows.length())
        {
            if (Xcbutills::getWindowDesktop(newwindows[i]) != currentdesk)
                newwindows.removeAt(i);
            else
                i++;
        }

        mainlayout->removeWidget(stretchwidget);

        i = 0;
        while (i < newwindows.length())
        {
            if (oldwindows.contains(newwindows[i]))
            {
                //winlist[newwindow[i]]->updatedata(X11UTILLS::getWindowIcon(newwindow[i]),X11UTILLS::getWindowTitle(newwindow[i]));
            }
            else
            {
                button *bt = new button(newwindows[i], Xcbutills::getWindowIcon(newwindows[i]), bttype, Xcbutills::getWindowTitle(newwindows[i]));
                connect(this, &windowlist::changehighlight, bt, &button::sethighlight);
                connect(this, &windowlist::closebts, bt, &button::closebt);
                connect(this, SIGNAL(updatebuttondata()), bt, SLOT(updatedata()));
                //connect(bt, &button::mouseEnter, ipopup, &imagepopup::btmouseEnter);
                //connect(bt, &button::mouseLeave, ipopup, &imagepopup::btmouseLeave);
                //connect(bt, &button::clicked, ipopup, &imagepopup::btclicked);
                mainlayout->addWidget(bt);
                winlist[newwindows[i]] = bt;
            }
            i++;
        }

        i = 0;
        while (i < oldwindows.length())
        {
            if (!newwindows.contains(oldwindows[i]))
            {
                mainlayout->removeWidget(winlist[oldwindows[i]]);
                winlist[oldwindows[i]]->close();
                delete winlist[oldwindows[i]];
            }
            i++;
        }

        resizebuttons(newwindows);

        //mainlayout->addWidget(stretchwidget, istretch);
        mainlayout->addWidget(stretchwidget, 1);
        oldwindows = newwindows;
    }
}

void windowlist::resizebuttons(QList<xcb_window_t> newwindow)
{
    if (bttype == "icononly")
    {
        //foreach(button *bt, winlist)
        //{

            //bt->setFixedWidth(bt->sizeHint().height());
        //}
        /*if (mainlayout->direction() == QBoxLayout::TopToBottom)
        {
            int c = 0;
            while (c < newwindow.length())
            {
                winlist[newwindow[c]]->setFixedHeight(26);
                c++;
            }
        }
        else
        {
            int c = 0;
            while (c < newwindow.length())
            {
                winlist[newwindow[c]]->setFixedWidth(26);
                c++;
            }
        }*/
    }
    else
    {
        if (mainlayout->direction() == QBoxLayout::TopToBottom)
        {
            int c = 0;
            while (c < newwindow.length())
            {
                winlist[newwindow[c]]->setFixedHeight(26);
                c++;
            }
        }
        else
        {
            if (newwindow.length() * maxbtsize < this->width())
            {
                int c = 0;
                while (c < newwindow.length())
                {
                    winlist[newwindow[c]]->setFixedWidth(maxbtsize);
                    c++;
                }
            }
            else
            {
                int newwidth = this->width() / newwindow.length();

                int c = 0;
                while (c < newwindow.length() - 1)
                {
                    winlist[newwindow[c]]->setFixedWidth(newwidth);
                    c++;
                }

                //resize last button to make it fill space exactly
                if (newwindow.length() * newwidth != this->width())
                {
                    int lastwidth = newwidth - (newwindow.length() * newwidth - this->width());
                    winlist[newwindow[c]]->setFixedWidth(lastwidth);
                }
                else
                {
                    winlist[newwindow[c]]->setFixedWidth(newwidth);
                }
            }
        }
    }
}

//process events from X11 i.e. active window change, active desktop change
//called from the plugin host
void windowlist::XcbEventFilter(xcb_generic_event_t* event)
{
    if (event->response_type == XCB_PROPERTY_NOTIFY)
    {
        xcb_client_message_event_t *message = reinterpret_cast<xcb_client_message_event_t *>(event);

        if (message->type == Xcbutills::atom("_NET_CLIENT_LIST")){
            updatelist();
        }
        if (message->type == Xcbutills::atom("_NET_ACTIVE_WINDOW")){
            emit changehighlight(Xcbutills::getActiveWindow());
        }
        if (message->type == Xcbutills::atom("_NET_CURRENT_DESKTOP")){
            int newcurrentdesk = Xcbutills::getCurrentDesktop();
            if (newcurrentdesk != currentdesk){
                currentdesk = newcurrentdesk;
                this->updatelist();
            }
        }
    }
}

void windowlist::reloadsettings()
{
    loadsettings();

    emit closebts();

    winlist.clear();
    oldwindows.clear();
    updatelist();
}

/*void windowlist::resetsize(QString panelpos)
{
    if (panelpos == "left")
        mainlayout->setDirection(QBoxLayout::TopToBottom);
    else if (panelpos == "right")
        mainlayout->setDirection(QBoxLayout::TopToBottom);
    else if (panelpos == "top")
        mainlayout->setDirection(QBoxLayout::LeftToRight);
    else
        mainlayout->setDirection(QBoxLayout::LeftToRight);

    emit closebts();

    winlist.clear();
    oldwindows.clear();
    updatelist();
}*/

void windowlist::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        pmenu->show();
}

void windowlist::loadsettings()
{
    QSettings settings("Forest", "Window List");
    settings.sync();
    bttype = settings.value("buttontype", "twopart").toString();
    maxbtsize = settings.value("maxbuttonsize", 150).toInt();
}

void windowlist::showsettingswidget()
{
    connect(swidget, SIGNAL(settingschanged()), this, SLOT(reloadsettings()));
    swidget->show();
}
