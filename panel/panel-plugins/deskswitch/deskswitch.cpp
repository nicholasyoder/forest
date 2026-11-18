// SPDX-License-Identifier: LGPL-3.0-or-later

#include "deskswitch.h"

deskswitch::deskswitch() {}

deskswitch::~deskswitch() {
    delete pmenu;
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

    workspaces = new WorkspaceTracker(this);
    connect(workspaces, &WorkspaceTracker::changed, this, &deskswitch::onWorkspacesChanged);
}

void deskswitch::setupbts(){
    dbuttons.clear();
    QLayoutItem *child;
    while ((child = basehlayout->takeAt(0)) != nullptr){
        delete child->widget();
        delete child;
    }

    for (int index = 0; index < workspaces->count(); index++){
        deskbutton *bt = new deskbutton(index);
        basehlayout->addWidget(bt);
        connect(bt, SIGNAL(clicked(int)), this, SLOT(switchtodesk(int)));
        connect(this, SIGNAL(activate(int)), bt, SLOT(setactive(int)));
        dbuttons << bt;
    }
}

void deskswitch::switchtodesk(int index){
    if (index < 0 || index >= workspaces->count())
        return;

    workspaces->activate(index);
    emit activate(index);
}

void deskswitch::onWorkspacesChanged(){
    if (workspaces->count() != dbuttons.length())
        setupbts();

    emit activate(workspaces->activeIndex());

    const QList<int> counts = workspaces->windowCounts();
    for (deskbutton *bt : std::as_const(dbuttons))
        bt->setNumDeskWindows(counts.value(bt->desknumber(), 0));
}
