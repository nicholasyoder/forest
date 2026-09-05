// SPDX-License-Identifier: LGPL-3.0-or-later

#include "windowbutton.h"

#include "extworkspacehandle.h"

#include <QDBusConnection>
#include <QDBusInterface>

namespace {
constexpr char kBiomeService[] = "org.biome";
constexpr char kWorkspacesPath[] = "/org/biome/Workspaces";
constexpr char kWorkspacesInterface[] = "org.biome.Workspaces";
}

struct MenuItem {
    QString text;
    QString icon;
    void (windowbutton::*slot)();
};

windowbutton::windowbutton(ForeignToplevelHandle *toplevelHandle, QIcon icon, QString text,
        ExtWorkspaceManager *workspaceManager)
    : handle(toplevelHandle), workspace_manager(workspaceManager){
    setupIconAndTextButton(text, icon);

    pmenu = new popupmenu(this, CenteredOnWidget);
    desk_menu = new popupmenu(this, CenteredOnWidget);

    // Clicking this closes pmenu (via popupmenu::additem's own auto-connect,
    // below) and opens desk_menu - the closest thing to a submenu Forest's
    // flat popupmenu supports. Matches the old X11 menu's ordering (this
    // item came first, before Raise/Maximize/etc.) and wording.
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

    if (workspace_manager->workspaces().isEmpty())
        connect(workspace_manager, &ExtWorkspaceManager::workspacesChanged, this, &windowbutton::populateDeskMenu);
    else
        populateDeskMenu();

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

void windowbutton::populateDeskMenu(){
    disconnect(workspace_manager, &ExtWorkspaceManager::workspacesChanged, this, &windowbutton::populateDeskMenu);

    const QList<ExtWorkspaceHandle*> workspaces = workspace_manager->workspaces();
    for (int index = 0; index < workspaces.length(); index++){
        // "Desktop " + 1-based number, matching the old X11 menu's wording
        // exactly rather than using Biome's own workspace name (which
        // happens to also just be the 1-based number today, but isn't
        // guaranteed to be).
        pmenuitem *item = new pmenuitem("Desktop " + QString::number(index + 1));
        connect(item, &pmenuitem::clicked, this, [this, index](){ moveToDesktop(index); });
        desk_menu->additem(item);
    }
}

void windowbutton::moveToDesktop(int workspace){
    // Pairing (see windowlist.cpp's tryPairPendingHandles()) normally
    // completes well before a user can right-click and choose this, but
    // there's no protocol guarantee of it - silently no-op rather than
    // send a bogus empty identifier to Biome.
    if (m_identifier.isEmpty())
        return;

    QDBusInterface iface(kBiomeService, kWorkspacesPath, kWorkspacesInterface, QDBusConnection::sessionBus());
    iface.call("MoveToplevelToWorkspace", m_identifier, workspace);
}
