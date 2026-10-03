// SPDX-License-Identifier: LGPL-3.0-or-later

#include "deskswitch.h"

#include "extworkspacehandle.h"

deskswitch::deskswitch() {}

deskswitch::~deskswitch() {
    delete pmenu;
    if (workspace_manager)
        workspace_manager->release();
}

void deskswitch::setupPlug(QBoxLayout *layout, QList<QAction*> itemlist)
{
    basehlayout = new QHBoxLayout(this);
    basehlayout->setContentsMargins(QMargins(0,0,0,0));
    basehlayout->setSpacing(0);

    layout->addWidget(this);

    pmenu = new QMenu;
    pmenu->addActions(itemlist);

    connect(this, &deskswitch::rightclicked, this, [this]{ popupMenuOnLauncher(pmenu, this, CenteredOnWidget); });

    workspace_manager = new ExtWorkspaceManager(this);
    connect(workspace_manager, &ExtWorkspaceManager::workspacesChanged, this, &deskswitch::onWorkspacesChanged);

    biome_workspaces = new BiomeWorkspaces(this);
    connect(biome_workspaces, &BiomeWorkspaces::windowWorkspacesChanged, this, &deskswitch::updateWindowCounts);
}

QHash<QString, QString> deskswitch::getpluginfo(){
    QHash<QString, QString> info;
    info["name"] = "Desktop Switcher";
    return info;
}

void deskswitch::setupbts(){
    dbuttons.clear();
    QLayoutItem *child;
    while ((child = basehlayout->takeAt(0)) != nullptr){
        delete child->widget();
        delete child;
    }

    const QList<ExtWorkspaceHandle*> workspaces = workspace_manager->workspaces();
    for (int index = 0; index < workspaces.length(); index++){
        deskbutton *bt = new deskbutton(index);
        basehlayout->addWidget(bt);
        connect(bt, SIGNAL(clicked(int)), this, SLOT(switchtodesk(int)));
        connect(this, SIGNAL(activate(int)), bt, SLOT(setactive(int)));
        dbuttons << bt;
    }
    updateWindowCounts();
}

void deskswitch::switchtodesk(int index){
    const QList<ExtWorkspaceHandle*> workspaces = workspace_manager->workspaces();
    if (index < 0 || index >= workspaces.length())
        return;

    workspaces[index]->activate();
    emit activate(index);
}

void deskswitch::onWorkspacesChanged(){
    const QList<ExtWorkspaceHandle*> workspaces = workspace_manager->workspaces();

    if (workspaces.length() != dbuttons.length())
        setupbts();

    emit activate(workspace_manager->activeWorkspaceIndex());
}

void deskswitch::updateWindowCounts(){
    QHash<int, int> counts;
    foreach (const QVariant &workspace, biome_workspaces->windowWorkspaces())
        counts[workspace.toInt()]++;

    foreach (deskbutton *bt, dbuttons)
        bt->setNumDeskWindows(counts.value(bt->desknumber(), 0));
}
