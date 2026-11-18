// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DESKSWITCH_H
#define DESKSWITCH_H

#include <QWidget>
#include <QMenu>
#include <QVBoxLayout>

#include "panelbutton.h"
#include "panelpluginterface.h"
#include "panelanchor.h"
#include "deskbutton.h"
#include "workspacetracker.h"

class deskswitch : public panelbutton, panelpluginterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.deskswitch.plugin" FILE "deskswitch.json")
    Q_INTERFACES(panelpluginterface)

public:
    deskswitch();
    ~deskswitch();

    //begin plugininterface
    void setupPlug(QBoxLayout *layout, QList<QAction*> itemlist);
    void closePlug(){this->close(); deleteLater();}
    //end plugininterface

signals:
    void activate(int desk);

private slots:
    void setupbts();
    void switchtodesk(int index);

    void onWorkspacesChanged();

private:
    QHBoxLayout *basehlayout;
    QList<deskbutton*> dbuttons;
    QMenu *pmenu = nullptr;
    WorkspaceTracker *workspaces = nullptr;
};

#endif // DESKSWITCH_H
