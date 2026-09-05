// SPDX-License-Identifier: LGPL-3.0-or-later

#include "deskswitch.h"

#include "extworkspacehandle.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

namespace {
constexpr char kBiomeService[] = "org.biome";
constexpr char kWorkspacesPath[] = "/org/biome/Workspaces";
constexpr char kWorkspacesInterface[] = "org.biome.Workspaces";
}

deskswitch::deskswitch() {}

deskswitch::~deskswitch() {}

void deskswitch::setupPlug(QBoxLayout *layout, QList<pmenuitem *> itemlist)
{
    basehlayout = new QHBoxLayout(this);
    basehlayout->setContentsMargins(QMargins(0,0,0,0));
    basehlayout->setSpacing(0);

    layout->addWidget(this);

    pmenu = new popupmenu(this, CenteredOnWidget);
    foreach (pmenuitem *item, itemlist)
        pmenu->additem(item);

    connect(this, &deskswitch::rightclicked, pmenu, &popupmenu::show);

    // No manual seeding needed: binding the manager makes the compositor
    // replay the full workspace list (workspace_group + one workspace per
    // index + done) immediately - see extworkspacemanager.h.
    workspace_manager = new ExtWorkspaceManager();
    connect(workspace_manager, &ExtWorkspaceManager::workspacesChanged, this, &deskswitch::onWorkspacesChanged);

    QDBusConnection::sessionBus().connect(kBiomeService, kWorkspacesPath, kWorkspacesInterface,
        "WindowWorkspacesChanged", this, SLOT(onWindowWorkspacesChanged(QVariantMap)));
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
    refreshWindowWorkspaces();
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

    // Biome's workspace count is fixed at startup, so this only rebuilds
    // buttons on the very first burst in practice - every later done()
    // (following a switch) just updates the active highlight below.
    if (workspaces.length() != dbuttons.length())
        setupbts();

    emit activate(workspace_manager->activeWorkspaceIndex());
}

void deskswitch::refreshWindowWorkspaces(){
    QDBusInterface iface(kBiomeService, kWorkspacesPath, kWorkspacesInterface, QDBusConnection::sessionBus());
    QDBusReply<QVariantMap> reply = iface.call("GetWindowWorkspaces");
    if (reply.isValid())
        applyWindowWorkspaces(reply.value());
}

void deskswitch::onWindowWorkspacesChanged(QVariantMap windowWorkspaces){
    applyWindowWorkspaces(windowWorkspaces);
}

// windowWorkspaces is identifier -> workspace index, one entry per open
// window (see org.biome.Workspaces.GetWindowWorkspaces) - tallied here into
// the per-desktop counts deskbutton actually displays.
void deskswitch::applyWindowWorkspaces(const QVariantMap &windowWorkspaces){
    QHash<int, int> counts;
    foreach (const QVariant &workspace, windowWorkspaces)
        counts[workspace.toInt()]++;

    foreach (deskbutton *bt, dbuttons)
        bt->setNumDeskWindows(counts.value(bt->desknumber(), 0));
}
