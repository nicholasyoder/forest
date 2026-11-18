// SPDX-License-Identifier: LGPL-3.0-or-later

#include "windowbutton.h"

#include "iconresolver.h"

struct MenuItem {
    QString text;
    QString icon;
    void (windowbutton::*slot)();
};

windowbutton::windowbutton(ForeignToplevelHandle *toplevelHandle, WorkspaceTracker *workspaceTracker)
    : panelbutton(IconAndText), handle(toplevelHandle), workspaces(workspaceTracker){
    m_appId = handle->appId();
    setText(handle->title());
    setIcon(iconresolver::iconForAppId(m_appId));
    setDown(handle->isActivated());

    pmenu = new QMenu;
    if (workspaces->canMoveWindows())
        desk_menu = pmenu->addMenu(QIcon::fromTheme("window-next"), "Move to workspace");

    MenuItem pmenu_items[] = {
        {"Raise", "arrow-up", &windowbutton::raise_w},
        {"Maximize", "arrow-up-double", &windowbutton::maximize_w},
        {"Demaximize", "arrow-down", &windowbutton::demaximize_w},
        {"Minimize", "arrow-down-double", &windowbutton::minimize_w},
        {"Close", "window-close", &windowbutton::close_w}
    };
    for (const auto& item : pmenu_items)
        pmenu->addAction(QIcon::fromTheme(item.icon), item.text, this, item.slot);

    if (workspaces->count() == 0)
        connect(workspaces, &WorkspaceTracker::changed, this, &windowbutton::populateDeskMenu);
    else
        populateDeskMenu();

    connect(this, &windowbutton::enterevent, this, &windowbutton::handleEnterEvent);
    connect(this, &windowbutton::leaveevent, this, &windowbutton::handleLeaveEvent);
}

void windowbutton::syncFromHandle(){
    setText(handle->title());
    if (handle->appId() != m_appId){
        m_appId = handle->appId();
        setIcon(iconresolver::iconForAppId(m_appId));
    }
    setDown(handle->isActivated());
}

void windowbutton::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton){
        dragActive = true;
        dragPos = event->pos();
    }
}

void windowbutton::mouseMoveEvent(QMouseEvent *event){
    // No drag-up-to-fit-on-screen: wlr-foreign-toplevel-management has no
    // move/resize/geometry requests.
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
        pmenu->setMinimumWidth(width());
        popupMenuOnLauncher(pmenu, this, CenteredOnWidget);
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

void windowbutton::populateDeskMenu(){
    if (workspaces->count() == 0)
        return; // the list hasn't arrived yet
    disconnect(workspaces, &WorkspaceTracker::changed, this, &windowbutton::populateDeskMenu);
    if (!desk_menu)
        return;

    for (int index = 0; index < workspaces->count(); index++){
        desk_menu->addAction("Workspace " + QString::number(index + 1), this, [this, index](){ moveToDesktop(index); });
    }
}

void windowbutton::moveToDesktop(int workspace){
    // Identifier is empty until paired with its ext-foreign-toplevel-list handle.
    workspaces->moveWindow(handle->identifier(), workspace);
}
