// SPDX-License-Identifier: LGPL-3.0-or-later

#include "windowlist.h"

windowlist::windowlist(){}

windowlist::~windowlist(){
    // Handles are the managers' children, freed once each manager finishes.
    if (toplevel_manager)
        toplevel_manager->release();
    if (ext_toplevel_list)
        ext_toplevel_list->release();
    if (workspace_manager)
        workspace_manager->release();
}

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

    biome_workspaces = new BiomeWorkspaces(this);
    connect(biome_workspaces, &BiomeWorkspaces::windowWorkspacesChanged, this, &windowlist::refreshVisibility);

    workspace_manager = new ExtWorkspaceManager(this);
    connect(workspace_manager, &ExtWorkspaceManager::workspacesChanged, this, &windowlist::refreshVisibility);

    toplevel_manager = new ForeignToplevelManager(this);
    connect(toplevel_manager, &ForeignToplevelManager::toplevelCreated, this, &windowlist::onWindowAdded);

    // Identifiers only matter to org.biome.Workspaces. Bound together with
    // toplevel_manager so both replays line up for pairing.
    if (biome_workspaces->isAvailable()){
        ext_toplevel_list = new ExtForeignToplevelList(this);
        connect(ext_toplevel_list, &ExtForeignToplevelList::toplevelCreated, this, &windowlist::onExtToplevelCreated);
    }
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
    connect(handle, &ForeignToplevelHandle::changed, this, &windowlist::onWindowChanged);
    connect(handle, &ForeignToplevelHandle::closed, this, &windowlist::onWindowRemoved);

    // Queued now, not on first `done`: pairing depends on creation order.
    if (ext_toplevel_list){
        pending_zwlr_handles << handle;
        tryPairPendingHandles();
    }
}

// The button is created on the first `done`, once title/app_id are known.
void windowlist::onWindowChanged(ForeignToplevelHandle *handle){
    if (windowbutton *wbt = button_list.value(handle)){
        wbt->syncFromHandle();
        return;
    }

    windowbutton *wbt = new windowbutton(handle, workspace_manager, biome_workspaces);
    connect(wbt, &windowbutton::moved, this, &windowlist::onButtonMoved);
    connect(wbt, &windowbutton::mouseEnter, ipopup, &imagepopup::btmouseEnter);
    connect(wbt, &windowbutton::mouseLeave, ipopup, &imagepopup::btmouseLeave);
    connect(wbt, &windowbutton::request_ipopup_close, ipopup, &imagepopup::closepopup);
    wbt->setMaximumWidth(maxbtsize);
    mainlayout->addWidget(wbt, 1);
    button_list[handle] = wbt;

    refreshVisibility();
}

void windowlist::onWindowRemoved(ForeignToplevelHandle *handle){
    pending_zwlr_handles.removeAll(handle);

    if (windowbutton *wbt = button_list.take(handle)){
        mainlayout->removeWidget(wbt);
        wbt->close();
        wbt->deleteLater();
    }
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

void windowlist::tryPairPendingHandles(){
    bool paired = false;
    while (!pending_zwlr_handles.isEmpty() && !pending_ext_handles.isEmpty()){
        QPointer<ForeignToplevelHandle> zwlrHandle = pending_zwlr_handles.takeFirst();
        QPointer<ExtForeignToplevelHandle> extHandle = pending_ext_handles.takeFirst();
        // QPointer: fail safe if a stale entry is ever re-queued.
        if (zwlrHandle && extHandle)
            zwlrHandle->setIdentifier(extHandle->identifier());
        if (extHandle)
            extHandle->deleteLater();
        paired = true;
    }
    if (paired)
        refreshVisibility();
}

// Only the active workspace's windows are shown. Unclassified windows
// (unpaired, or not yet reported by Biome) default to visible.
void windowlist::refreshVisibility(){
    int active = workspace_manager->activeWorkspaceIndex();
    if (active < 0)
        return;

    const QVariantMap &windowWorkspaces = biome_workspaces->windowWorkspaces();
    foreach (windowbutton *wbt, button_list){
        const QString identifier = wbt->toplevelHandle()->identifier();
        bool visible = !windowWorkspaces.contains(identifier) || windowWorkspaces.value(identifier).toInt() == active;
        wbt->setVisible(visible);
    }
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
