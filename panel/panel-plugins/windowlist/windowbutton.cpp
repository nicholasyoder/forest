// SPDX-License-Identifier: LGPL-3.0-or-later

#include "windowbutton.h"

struct MenuItem {
    QString text;
    QString icon;
    void (windowbutton::*slot)();
};

windowbutton::windowbutton(ForeignToplevelHandle *toplevelHandle, QIcon icon, QString text) : handle(toplevelHandle){
    setupIconAndTextButton(text, icon);

    pmenu = new popupmenu(this, CenteredOnWidget);

    // "Move to desktop" is disabled until Workstream D exposes a workspace
    // protocol - see the comment on desk_menu in windowbutton.h. Left
    // unpopulated (no per-desktop items to build without
    // Xcbutills::getNumDesktops()/moveWindowToDesktop()) and never shown.
    desk_menu = new popupmenu(this, CenteredOnWidget);

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
    // Drag-up-to-fit-on-screen (the old Xcbutills::fitWindowOnScreen path)
    // is dropped: wlr-foreign-toplevel-management has no move/resize/
    // geometry requests at all, so there's nothing to port it to.
    if (dragActive){
        if(event->pos().x() < -5){
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
    handle->activate();
}

void windowbutton::maximize_w(){
    handle->setMaximized();
}

void windowbutton::minimize_w(){
    handle->setMinimized();
}

void windowbutton::close_w(){
    handle->requestClose();
}

void windowbutton::demaximize_w(){
    handle->unsetMaximized();
}
