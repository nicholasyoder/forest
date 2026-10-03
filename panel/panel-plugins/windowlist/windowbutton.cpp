// SPDX-License-Identifier: LGPL-3.0-or-later

#include "windowbutton.h"

#include "extworkspacehandle.h"
#include "iconresolver.h"

struct MenuItem {
    QString text;
    QString icon;
    void (windowbutton::*slot)();
};

windowbutton::windowbutton(ForeignToplevelHandle *toplevelHandle, ExtWorkspaceManager *workspaceManager,
        BiomeWorkspaces *biomeWorkspaces)
    : panelbutton(IconAndText), handle(toplevelHandle), workspace_manager(workspaceManager), biome_workspaces(biomeWorkspaces){
    m_appId = handle->appId();
    setText(handle->title());
    setIcon(iconresolver::iconForAppId(m_appId));
    setDown(handle->isActivated());

    pmenu = new QMenu;
    if (biome_workspaces->isAvailable())
        desk_menu = pmenu->addMenu(QIcon::fromTheme("window-next"), "Move to desktop");

    MenuItem pmenu_items[] = {
        {"Raise", "arrow-up", &windowbutton::raise_w},
        {"Maximize", "arrow-up-double", &windowbutton::maximize_w},
        {"Demaximize", "arrow-down", &windowbutton::demaximize_w},
        {"Minimize", "arrow-down-double", &windowbutton::minimize_w},
        {"Close", "window-close", &windowbutton::close_w}
    };
    for (const auto& item : pmenu_items)
        pmenu->addAction(QIcon::fromTheme(item.icon), item.text, this, item.slot);

    if (workspace_manager->workspaces().isEmpty())
        connect(workspace_manager, &ExtWorkspaceManager::workspacesChanged, this, &windowbutton::populateDeskMenu);
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
    disconnect(workspace_manager, &ExtWorkspaceManager::workspacesChanged, this, &windowbutton::populateDeskMenu);
    if (!desk_menu)
        return;

    const QList<ExtWorkspaceHandle*> workspaces = workspace_manager->workspaces();
    for (int index = 0; index < workspaces.length(); index++){
        desk_menu->addAction("Desktop " + QString::number(index + 1), this, [this, index](){ moveToDesktop(index); });
    }
}

void windowbutton::moveToDesktop(int workspace){
    // Empty until paired with its ext-foreign-toplevel-list handle.
    if (!handle->identifier().isEmpty())
        biome_workspaces->moveToplevel(handle->identifier(), workspace);
}
