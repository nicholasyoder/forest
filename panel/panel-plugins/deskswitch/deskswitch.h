// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DESKSWITCH_H
#define DESKSWITCH_H

#include <QWidget>
#include <QVBoxLayout>
#include <QVariantMap>

#include "panelbutton.h"
#include "panelpluginterface.h"
#include "deskbutton.h"
#include "extworkspacemanager.h"

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
    void XcbEventFilter(xcb_generic_event_t*){}
    QHash<QString, QString> getpluginfo();
    //end plugininterface

signals:
    void activate(int desk);

private slots:
    void setupbts();
    void switchtodesk(int index);

    // Active-desktop highlighting: driven by ext-workspace-v1's own state,
    // fired after every workspacesChanged() (initial burst, and again after
    // every subsequent activate()/commit() round trip).
    void onWorkspacesChanged();

    // Per-desktop window-count dots: driven independently, from
    // org.biome.Workspaces over DBus - ext-workspace-v1 has no concept of
    // which windows belong to which workspace at all (see
    // biome/docs/phase4-plan.md Workstream D). GetWindowWorkspaces/
    // WindowWorkspacesChanged hand back identifier -> workspace index for
    // every open window; tallied into per-desktop counts here.
    void refreshWindowWorkspaces();
    void onWindowWorkspacesChanged(QVariantMap windowWorkspaces);

private:
    void applyWindowWorkspaces(const QVariantMap &windowWorkspaces);

    QHBoxLayout *basehlayout;
    QList<deskbutton*> dbuttons;
    popupmenu *pmenu;
    ExtWorkspaceManager *workspace_manager = nullptr;
};

#endif // DESKSWITCH_H
