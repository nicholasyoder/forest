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

windowlist::windowlist(){}

windowlist::~windowlist(){}

void windowlist::setupPlug(QBoxLayout *layout, QList<pmenuitem *> itemlist){
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
    //there should be a better way to do this instead of running a timer all the time
    QTimer *t = new QTimer;
    connect(t, SIGNAL(timeout()), this, SIGNAL(updatebuttondata()));
    t->start(800);
}

QHash<QString, QString> windowlist::getpluginfo(){
    QHash<QString, QString> info;
    info["name"] = "Window List";
    info["needsXcbEvents"] = "true";
    info["stretch"] = "true";
    return info;
}

void windowlist::updatelist(){
    QList<xcb_window_t> x_client_list = Xcbutills::getClientList();

    if (x_client_list != oldwindows || currentdesk != olddesk){
        mainlayout->removeWidget(stretchwidget);

        QList<xcb_window_t> desk_windows;
        foreach (xcb_window_t window, x_client_list) {
            if (Xcbutills::getWindowDesktop(window) == currentdesk){
                desk_windows.append(window);

                if (!oldwindows.contains(window)){
                    button *bt = new button(window, Xcbutills::getWindowIcon(window), bttype, Xcbutills::getWindowTitle(window));
                    connect(this, &windowlist::changehighlight, bt, &button::sethighlight);
                    connect(this, &windowlist::updatebuttondata, bt, &button::updatedata);
                    //connect(bt, &button::mouseEnter, ipopup, &imagepopup::btmouseEnter);
                    //connect(bt, &button::mouseLeave, ipopup, &imagepopup::btmouseLeave);
                    //connect(bt, &button::clicked, ipopup, &imagepopup::btclicked);

                    bt->setMaximumWidth(maxbtsize);
                    mainlayout->addWidget(bt, 1);
                    button_list[window] = bt;
                }
            }
        }

        foreach (xcb_window_t window, oldwindows) {
            if (!desk_windows.contains(window)) {
                mainlayout->removeWidget(button_list[window]);
                button_list[window]->close();
                delete button_list[window];
            }
        }

        mainlayout->addWidget(stretchwidget, 0);
        oldwindows = desk_windows;
        olddesk = currentdesk;
    }
}

//process events from X11 i.e. active window change, active desktop change
//called from the plugin host
void windowlist::XcbEventFilter(xcb_generic_event_t* event){
    if (event->response_type == XCB_PROPERTY_NOTIFY){
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
                updatelist();
            }
        }
    }
}

void windowlist::reloadsettings(){
    loadsettings();
    foreach (unsigned long key, button_list.keys()) {
        button *bt = button_list[key];
        if (bt){ bt->close(); bt->deleteLater(); }
    }
    button_list.clear();
    oldwindows.clear();
    updatelist();
}

void windowlist::mouseReleaseEvent(QMouseEvent *event){
    if (event->button() == Qt::RightButton)
        pmenu->show();
}

void windowlist::loadsettings(){
    QSettings settings("Forest", "Window List");
    settings.sync();
    bttype = settings.value("buttontype", "twopart").toString();
    maxbtsize = settings.value("maxbuttonsize", 150).toInt();
}

void windowlist::showsettingswidget(){
    connect(swidget, SIGNAL(settingschanged()), this, SLOT(reloadsettings()));
    swidget->show();
}
