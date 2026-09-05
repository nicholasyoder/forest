// SPDX-License-Identifier: LGPL-3.0-or-later

#include "windowlist.h"

#include "iconresolver.h"

namespace {
constexpr char kBiomeService[] = "org.biome";
constexpr char kWorkspacesPath[] = "/org/biome/Workspaces";
constexpr char kWorkspacesInterface[] = "org.biome.Workspaces";
}

windowlist::windowlist(){}

windowlist::~windowlist(){}

void windowlist::setupPlug(QBoxLayout *layout, QList<pmenuitem *> itemlist){
    layout->addWidget(this);

    QHBoxLayout *baseLayout = new QHBoxLayout;
    baseLayout->addLayout(mainlayout);
    baseLayout->setContentsMargins(QMargins(0,0,0,0));
    baseLayout->setSpacing(0);
    mainlayout->setContentsMargins(QMargins(0,0,0,0));
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

    // No manual seeding step needed here: binding the manager makes the
    // compositor replay a `toplevel` event for every already-open window,
    // same as it does for windows opened afterward.
    toplevel_manager = new ForeignToplevelManager();
    connect(toplevel_manager, &ForeignToplevelManager::toplevelCreated, this, &windowlist::onWindowAdded);

    ext_toplevel_list = new ExtForeignToplevelList();
    connect(ext_toplevel_list, &ExtForeignToplevelList::toplevelCreated, this, &windowlist::onExtToplevelCreated);

    workspace_manager = new ExtWorkspaceManager();
    // Re-filter on every desktop switch (as well as the initial burst,
    // which is when workspace_manager->activeWorkspaceIndex() first becomes
    // valid) - see refreshVisibility().
    connect(workspace_manager, &ExtWorkspaceManager::workspacesChanged, this, &windowlist::refreshVisibility);

    refreshWindowWorkspaces();
    QDBusConnection::sessionBus().connect(kBiomeService, kWorkspacesPath, kWorkspacesInterface,
        "WindowWorkspacesChanged", this, SLOT(onWindowWorkspacesChanged(QVariantMap)));
}

QHash<QString, QString> windowlist::getpluginfo(){
    QHash<QString, QString> info;
    info["name"] = "Window List";
    info["stretch"] = "true";
    return info;
}

void windowlist::reloadsettings(){
    loadsettings();
    foreach (windowbutton *wbt, button_list) {
        wbt->setMaximumWidth(maxbtsize);
    }
}

void windowlist::mouseReleaseEvent(QMouseEvent *event){
    if (event->button() == Qt::RightButton)
        pmenu->show();
}

void windowlist::loadsettings(){
    QSettings settings("Forest", "Window List");
    settings.sync();
    ipopup->set_enabled(settings.value("showthumbnails", true).toBool());
    maxbtsize = settings.value("maxbuttonsize", 170).toInt();
}

void windowlist::showsettingswidget(){
    connect(swidget, SIGNAL(settingschanged()), this, SLOT(reloadsettings()));
    swidget->show();
}


void windowlist::onWindowAdded(ForeignToplevelHandle *handle){
    // No filtering: wlr-foreign-toplevel-management has no window-type/
    // skip-taskbar concept, so unlike the old KWindowInfo-based
    // acceptWindow(), every toplevel the compositor reports gets a button.
    if (button_list.contains(handle))
        return;

    connect(handle, &ForeignToplevelHandle::changed, this, &windowlist::onWindowChanged);
    connect(handle, &ForeignToplevelHandle::closed, this, &windowlist::onWindowRemoved);

    windowbutton *wbt = new windowbutton(handle, iconresolver::iconForAppId(handle->appId()), handle->title(), workspace_manager);
    connect(wbt, &windowbutton::moved, this, &windowlist::onButtonMoved);
    connect(wbt, &windowbutton::mouseEnter, ipopup, &imagepopup::btmouseEnter);
    connect(wbt, &windowbutton::mouseLeave, ipopup, &imagepopup::btmouseLeave);
    connect(wbt, &windowbutton::request_ipopup_close, ipopup, &imagepopup::closepopup);
    wbt->setMaximumWidth(maxbtsize);
    mainlayout->addWidget(wbt, 1);
    button_list[handle] = wbt;

    pending_zwlr_handles << handle;
    tryPairPendingHandles();

    // window_workspaces won't have this window's entry yet at this exact
    // point (it's keyed by ext-foreign-toplevel-list identifier, which may
    // not even be paired yet - see tryPairPendingHandles() above), but
    // Biome's own WindowWorkspacesChanged signal (fired synchronously from
    // foreign_toplevel_create()) should arrive and correct this on its own
    // almost immediately. This call is just to avoid a visible flash on a
    // non-active desktop in the meantime - refreshVisibility() defaults an
    // unclassified window to visible, which is only wrong for a window
    // that maps directly onto an inactive workspace, a rare/brief case.
    refreshVisibility();
}

void windowlist::onWindowRemoved(ForeignToplevelHandle *handle){
    if(!button_list.contains(handle))
        return;

    pending_zwlr_handles.removeAll(handle);

    windowbutton *wbt = button_list[handle];
    button_list.remove(handle);
    mainlayout->removeWidget(wbt);
    wbt->close();
    wbt->deleteLater();
    handle->deleteLater();
}

void windowlist::onExtToplevelCreated(ExtForeignToplevelHandle *handle){
    connect(handle, &ExtForeignToplevelHandle::ready, this, &windowlist::onExtToplevelReady);
    connect(handle, &ExtForeignToplevelHandle::closed, this, &windowlist::onExtToplevelClosed);
}

void windowlist::onExtToplevelReady(ExtForeignToplevelHandle *handle){
    pending_ext_handles << handle;
    tryPairPendingHandles();
}

void windowlist::onExtToplevelClosed(ExtForeignToplevelHandle *handle){
    pending_ext_handles.removeAll(handle);
    handle->deleteLater();
}

// See extforeigntoplevellist.h: Biome creates the wlr_ and ext_ handle for
// a given toplevel back-to-back, so the two managers' `toplevel` events
// arrive in the same relative order - simple front-of-both-queues pairing
// is enough, with no shared key needed between the two protocols.
void windowlist::tryPairPendingHandles(){
    while (!pending_zwlr_handles.isEmpty() && !pending_ext_handles.isEmpty()){
        ForeignToplevelHandle *zwlrHandle = pending_zwlr_handles.takeFirst();
        ExtForeignToplevelHandle *extHandle = pending_ext_handles.takeFirst();
        if (button_list.contains(zwlrHandle))
            button_list[zwlrHandle]->setIdentifier(extHandle->identifier());
        extHandle->deleteLater();
    }
}

void windowlist::refreshWindowWorkspaces(){
    QDBusInterface iface(kBiomeService, kWorkspacesPath, kWorkspacesInterface, QDBusConnection::sessionBus());
    QDBusReply<QVariantMap> reply = iface.call("GetWindowWorkspaces");
    if (reply.isValid())
        onWindowWorkspacesChanged(reply.value());
}

void windowlist::onWindowWorkspacesChanged(QVariantMap windowWorkspaces){
    window_workspaces = windowWorkspaces;
    refreshVisibility();
}

// Shows only the buttons whose window is on the active workspace - see
// this header's doc comment on refreshVisibility(). A button whose
// identifier isn't in window_workspaces yet (still pairing, or Biome
// hasn't reported it back yet) defaults to visible rather than hidden, so
// a brand-new window doesn't flash out of existence for a moment.
void windowlist::refreshVisibility(){
    int active = workspace_manager->activeWorkspaceIndex();
    if (active < 0)
        return;

    foreach (windowbutton *wbt, button_list){
        const QString identifier = wbt->identifier();
        bool visible = !window_workspaces.contains(identifier) || window_workspaces.value(identifier).toInt() == active;
        wbt->setVisible(visible);
    }
}

void windowlist::onWindowChanged(ForeignToplevelHandle *handle){
    if(!button_list.contains(handle))
        return;

    windowbutton *wbt = button_list[handle];
    wbt->setText(handle->title());
    wbt->setIcon(iconresolver::iconForAppId(handle->appId()));
    // Each handle already knows whether it itself is active, so no
    // separate "current active window" query/tracking is needed here the
    // way Xcbutills::getActiveWindow() required.
    wbt->setDown(handle->isActivated());
}

void windowlist::onButtonMoved(windowbutton *wbt, bool left){
    int index = mainlayout->indexOf(wbt);

    if (left){
        if (index != 0){
            QLayoutItem *item = mainlayout->takeAt(index);
            mainlayout->insertWidget(index-1, item->widget(), 1);
        }
    }
    else{
        if (index != mainlayout->count() - 1){
            QLayoutItem *item = mainlayout->takeAt(index);
            mainlayout->insertWidget(index+1, item->widget(), 1);
        }
    }
}
