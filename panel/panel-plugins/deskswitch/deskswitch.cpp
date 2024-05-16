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

#include "deskswitch.h"

deskswitch::deskswitch()
{

}

deskswitch::~deskswitch()
{
}

void deskswitch::setupPlug(QBoxLayout *layout, QList<pmenuitem *> itemlist)
{
    basehlayout = new QHBoxLayout(this);
    basehlayout->setMargin(0);
    basehlayout->setSpacing(0);

    layout->addWidget(this);

    deskcount = Xcbutills::getNumDesktops();
    //currentdesk = Xcbutills::getCurrentDesktop();

    setupbts();

    pmenu = new popupmenu(this, CenteredOnWidget);
    foreach (pmenuitem *item, itemlist)
        pmenu->additem(item);

    //pmenu->addseperator();
    //pmenuitem *item = new pmenuitem("Clock Settings", QIcon::fromTheme("configure"));
    //connect(item, &pmenuitem::clicked, this, &deskswitch::showsettingswidget);
    //pmenu->additem(item);

    connect(this, &deskswitch::rightclicked, pmenu, &popupmenu::show);
}

void deskswitch::XcbEventFilter(xcb_generic_event_t *event)
{
    if (event->response_type == 28)//PropertyNotify
    {
        int newcount = Xcbutills::getNumDesktops();
        if (newcount != deskcount)
        {
            deskcount = newcount;
            setupbts();
        }

        /*int newcurrentdesk = Xcbutills::getCurrentDesktop();
        if (newcurrentdesk != currentdesk)
        {
            currentdesk = newcurrentdesk;
            emit activate(currentdesk);

            QList<xcb_window_t> windows = Xcbutills::getClientList();
            QHash<int, QList<xcb_window_t>> deskwindows;
            foreach (xcb_window_t window, windows)
                deskwindows[Xcbutills::getWindowDesktop(window)] << window;

            foreach (deskbutton *bt, dbuttons)
                bt->setNumDeskWindows(deskwindows[bt->desknumber()].length());
        }*/


        xcb_client_message_event_t *message = reinterpret_cast<xcb_client_message_event_t *>(event);

        if (message->type == Xcbutills::atom("_NET_CLIENT_LIST")){
            QList<xcb_window_t> windows = Xcbutills::getClientList();
            QHash<int, QList<xcb_window_t>> deskwindows;
            foreach (xcb_window_t window, windows)
                deskwindows[Xcbutills::getWindowDesktop(window)] << window;

            foreach (deskbutton *bt, dbuttons)
                bt->setNumDeskWindows(deskwindows[bt->desknumber()].length());
        }
        else if (message->type == Xcbutills::atom("_NET_CURRENT_DESKTOP")){
            emit activate(Xcbutills::getCurrentDesktop());
        }
    }
}

QHash<QString, QString> deskswitch::getpluginfo()
{
    QHash<QString, QString> info;
    info["name"] = "Desktop Switcher";
    info["needsXcbEvents"] = "true";
    return info;
}

void deskswitch::setupbts()
{
    dbuttons.clear();
    QLayoutItem *child;
    while ((child = basehlayout->takeAt(0)) != nullptr)
    {
        delete child->widget();
        delete child;
    }

    int c = 1;
    while (c <= deskcount)
    {
        deskbutton *bt = new deskbutton(c);
        basehlayout->addWidget(bt);
        connect(bt, SIGNAL(clicked(int)), this, SLOT(switchtodesk(int)));
        connect(this, SIGNAL(activate(int)), bt, SLOT(setactive(int)));
        dbuttons << bt;
        c++;
    }

    emit activate(Xcbutills::getCurrentDesktop());
    //emit activate(currentdesk);
}

void deskswitch::switchtodesk(int num)
{
    Xcbutills::setCurrentDesktop(num);
    emit activate(num);
}
