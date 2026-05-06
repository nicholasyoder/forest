// SPDX-License-Identifier: LGPL-3.0-or-later

#include "windowbutton.h"

#include "xcbutills/xcbutills.h"

struct MenuItem {
    QString text;
    QString icon;
    void (windowbutton::*slot)();
};

windowbutton::windowbutton(ulong windowid, int desktop, QIcon icon, QString text) : window_id(windowid), window_desktop(desktop){
    setupIconAndTextButton(text, icon);

    pmenu = new popupmenu(this, CenteredOnWidget);
    desk_menu = new popupmenu(this, CenteredOnWidget);

    pmenuitem *desk_item = new pmenuitem("Move to desktop", QIcon::fromTheme("window-next"));
    connect(desk_item, &pmenuitem::clicked, desk_menu, &popupmenu::show);
    pmenu->additem(desk_item);

    MenuItem pmenu_items[] = {
        {"Raise", "arrow-up", &windowbutton::raise_w},
        {"Maximize", "arrow-up-double", &windowbutton::maximize_w},
        {"Demaximize", "arrow-down", &windowbutton::demaximize_w},
        {"Minimize", "arrow-down-double", &windowbutton::minimize_w},
        {"Close", "window-close", &windowbutton::close_w}
    };
    for (const auto& item : pmenu_items) {
        pmenuitem *menu_item = new pmenuitem(item.text, QIcon::fromTheme(item.icon));
        connect(menu_item, &pmenuitem::clicked, this, item.slot);
        pmenu->additem(menu_item);
    }

    for(int i = 1; i <= Xcbutills::getNumDesktops(); i++){
        pmenuitem *item = new pmenuitem("Desktop " + QString::number(i));
        connect(item, &pmenuitem::clicked, this, [this, i](){Xcbutills::moveWindowToDesktop(window_id, i);});
        desk_menu->additem(item);
    }

    connect(this, &windowbutton::enterevent, this, &windowbutton::handleEnterEvent);
    connect(this, &windowbutton::leaveevent, this, &windowbutton::handleLeaveEvent);
}

void windowbutton::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton){
        dragActive = true;
        dragPos = event->pos();
    }
}

void windowbutton::mouseMoveEvent(QMouseEvent *event){
    if (dragActive){
        if(event->pos().y() < 0){
            Xcbutills::fitWindowOnScreen(window_id);
            dragActive = false;
            allowReleaseAction = false;
        }
        else if(event->pos().x() < -5){
            emit moved(this, true);
            allowReleaseAction = false;
        }
        else if(event->pos().x() > this->width() + 5){
            emit moved(this, false);
            allowReleaseAction = false;
        }
    }
}

void windowbutton::mouseReleaseEvent(QMouseEvent *event){
    panelbutton::mouseReleaseEvent(event);
    dragActive = false;

    if(!allowReleaseAction){
        allowReleaseAction = true;
        return;
    }

    emit request_ipopup_close();

    if (event->button() == Qt::LeftButton){
        raise_w();
    }
    else if(event->button() == Qt::RightButton){
        QSize sizeHint = pmenu->popupw->sizeHint();
        if (width() > sizeHint.width())
            pmenu->popupw->setFixedSize(width(), sizeHint.height());
        else
            pmenu->popupw->setFixedSize(sizeHint.width(), sizeHint.height());

        pmenu->show();
    }
}

void windowbutton::raise_w(){
    Xcbutills::raiseWindow(window_id);
}

void windowbutton::maximize_w(){
    Xcbutills::maximizeWindow(window_id);
}

void windowbutton::minimize_w(){
    Xcbutills::minimizeWindow(window_id);
}

void windowbutton::close_w(){
    Xcbutills::closeWindow(window_id);
}

void windowbutton::demaximize_w(){
    Xcbutills::demaximizeWindow(window_id);
}
