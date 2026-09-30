// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DESKSWITCH_H
#define DESKSWITCH_H

#include <QWidget>
#include <QVBoxLayout>

#include "panelbutton.h"
#include "panelpluginterface.h"
#include "deskbutton.h"
#include "extworkspacemanager.h"
#include "biomeworkspaces.h"

class deskswitch : public panelbutton, panelpluginterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.deskswitch.plugin")
    Q_INTERFACES(panelpluginterface)

public:
    deskswitch();
    ~deskswitch();

    //begin plugininterface
    void setupPlug(QBoxLayout *layout, QList<pmenuitem*> itemlist);
    void closePlug(){this->close(); deleteLater();}
    QHash<QString, QString> getpluginfo();
    //end plugininterface

signals:
    void activate(int desk);

private slots:
    void setupbts();
    void switchtodesk(int index);

    void onWorkspacesChanged();
    // Per-desktop window-count dots.
    void updateWindowCounts();

private:
    QHBoxLayout *basehlayout;
    QList<deskbutton*> dbuttons;
    popupmenu *pmenu;
    ExtWorkspaceManager *workspace_manager = nullptr;
    BiomeWorkspaces *biome_workspaces = nullptr;
};

#endif // DESKSWITCH_H
