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


#include <KWindowInfo>

windowlist::windowlist(){}

windowlist::~windowlist(){}

void windowlist::setupPlug(QBoxLayout *layout, QList<pmenuitem *> itemlist){
    layout->addWidget(this);

    QHBoxLayout *baseLayout = new QHBoxLayout;
    baseLayout->addLayout(mainlayout);
    baseLayout->setMargin(0);
    baseLayout->setSpacing(0);
    mainlayout->setMargin(0);
    mainlayout->setSpacing(0);
    QWidget *swidget = new QWidget;
    baseLayout->addWidget(swidget);
    this->setLayout(baseLayout);

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
    //updatelist();

    //this updates the text on the button (for like when you change directories in a file manager & the window title changes too.)
    //there should be a better way to do this instead of running a timer all the time
    //QTimer *t = new QTimer;
    //connect(t, SIGNAL(timeout()), this, SIGNAL(updatebuttondata()));
    //t->start(800);

    connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, &windowlist::onWindowAdded);
    connect(KWindowSystem::self(), &KWindowSystem::windowRemoved, this, &windowlist::onWindowRemoved);
    connect(KWindowSystem::self(), static_cast<void (KWindowSystem::*)(WId, NET::Properties, NET::Properties2)>(&KWindowSystem::windowChanged),
            this, &windowlist::onWindowChanged);
    connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, &windowlist::onDesktopChanged);
}

QHash<QString, QString> windowlist::getpluginfo(){
    QHash<QString, QString> info;
    info["name"] = "Window List";
    info["needsXcbEvents"] = "true";
    info["stretch"] = "true";
    return info;
}

//void windowlist::updatelist(){
/*    QList<xcb_window_t> x_client_list = Xcbutills::getClientList();

    if (x_client_list != oldwindows || currentdesk != olddesk){
        mainlayout->removeWidget(stretchwidget);

        QList<xcb_window_t> desk_windows;
        foreach (xcb_window_t window, x_client_list) {
            if (Xcbutills::getWindowDesktop(window) == currentdesk){
                desk_windows.append(window);

                if (!oldwindows.contains(window)){

                    KWindowInfo info(window, NET::WMVisibleName | NET::WMName);
                    QString title = info.visibleName().isEmpty() ? info.name() : info.visibleName();

                    button *bt = new button(window, Xcbutills::getWindowIcon(window), bttype, title);
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
    }*/
//}

//process events from X11 i.e. active window change, active desktop change
//called from the plugin host
void windowlist::XcbEventFilter(xcb_generic_event_t* event){
/*    if (event->response_type == XCB_PROPERTY_NOTIFY){
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
    }*/
}

void windowlist::reloadsettings(){
    loadsettings();
    /*foreach (unsigned long key, button_list.keys()) {
        button *bt = button_list[key];
        if (bt){ bt->close(); bt->deleteLater(); }
    }*/
    //button_list.clear();
    //oldwindows.clear();
    //updatelist();
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


bool windowlist::acceptWindow(WId window) const
{
    QFlags<NET::WindowTypeMask> ignoreList;
    ignoreList |= NET::DesktopMask;
    ignoreList |= NET::DockMask;
    ignoreList |= NET::SplashMask;
    ignoreList |= NET::ToolbarMask;
    ignoreList |= NET::MenuMask;
    ignoreList |= NET::PopupMenuMask;
    ignoreList |= NET::NotificationMask;

    KWindowInfo info(window, NET::WMWindowType | NET::WMState, NET::WM2TransientFor);
    if (!info.valid())
        return false;

    if (NET::typeMatchesMask(info.windowType(NET::AllTypesMask), ignoreList))
        return false;

    if (info.state() & NET::SkipTaskbar)
        return false;

    // WM_TRANSIENT_FOR hint not set - normal window
    WId transFor = info.transientFor();
    if (transFor == 0 || transFor == window || transFor == (WId) QX11Info::appRootWindow())
        return true;

    info = KWindowInfo(transFor, NET::WMWindowType);

    QFlags<NET::WindowTypeMask> normalFlag;
    normalFlag |= NET::NormalMask;
    normalFlag |= NET::DialogMask;
    normalFlag |= NET::UtilityMask;

    return !NET::typeMatchesMask(info.windowType(NET::AllTypesMask), normalFlag);
}

void windowlist::onWindowAdded(WId window){
    if (!acceptWindow(window)) return;

    KWindowInfo info(window, NET::WMDesktop);
    int desktop = info.desktop();

    if(!button_list.contains(window)){
        windowbutton *wbt = new windowbutton(window, desktop, Xcbutills::getWindowIcon(window), Xcbutills::getWindowTitle(window));
        wbt->setMaximumWidth(maxbtsize);
        mainlayout->addWidget(wbt, 1);
        button_list[window] = wbt;

        if(desktop != currentdesk){
            wbt->setHidden(true);
        }
    }
}

void windowlist::onWindowRemoved(WId window){
    if(!button_list.contains(window))
        return;

    windowbutton *wbt = button_list[window];
    button_list.remove(window);
    mainlayout->removeWidget(wbt);
    wbt->close();
    wbt->deleteLater();
}

void windowlist::onWindowChanged(WId window, NET::Properties prop, NET::Properties2 prop2){
    if(button_list.contains(window)){
        windowbutton *wbt = button_list[window];
        if (prop.testFlag(NET::WMVisibleName) || prop.testFlag(NET::WMName))
            wbt->setText(Xcbutills::getWindowTitle(window));

        if (prop.testFlag(NET::WMIcon) || prop2.testFlag(NET::WM2WindowClass))
            wbt->setIcon(Xcbutills::getWindowIcon(window));

        if (prop.testFlag(NET::WMDesktop) || prop.testFlag(NET::WMGeometry)) {
            wbt->setWindowDesktop(Xcbutills::getWindowDesktop(window));
            wbt->setHidden(wbt->windowDesktop() != currentdesk);
        }

        if (prop.testFlag(NET::WMState)) {
            WId new_active_w = Xcbutills::getActiveWindow();
            if(new_active_w != active_window){
                if(button_list.contains(active_window))
                    button_list[active_window]->setDown(false);
                if(button_list.contains(new_active_w))
                    button_list[new_active_w]->setDown(true);
                active_window = new_active_w;
            }

            KWindowInfo info(window, NET::WMState);
            if (info.hasState(NET::SkipTaskbar)) {
                onWindowRemoved(window);
            }
        }
    }
}

void windowlist::onDesktopChanged(int desktop){
    currentdesk = desktop;
    foreach(windowbutton *wbt, button_list){
        wbt->setHidden(wbt->windowDesktop() != currentdesk);
    }
}
