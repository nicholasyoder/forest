// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WINDOWBUTTON_H
#define WINDOWBUTTON_H

#include <QMenu>

#include "panelbutton.h"
#include "panelanchor.h"
#include "foreigntoplevelhandle.h"
#include "extworkspacemanager.h"
#include "biomeworkspaces.h"

class windowbutton : public panelbutton{
    Q_OBJECT

public:
    windowbutton(ForeignToplevelHandle *handle, ExtWorkspaceManager *workspaceManager, BiomeWorkspaces *biomeWorkspaces);
    ~windowbutton() override { delete pmenu; }

    ForeignToplevelHandle *toplevelHandle(){return handle;}

    // Updates text/icon/state from the handle; the icon is only re-resolved
    // when app_id changes.
    void syncFromHandle();

signals:
    void moved(windowbutton *wbt, bool left);

    void mouseEnter(windowbutton *wbt);
    void mouseLeave(windowbutton *wbt);
    void request_ipopup_close();

private slots:
    void handleEnterEvent(){emit mouseEnter(this);}
    void handleLeaveEvent(){emit mouseLeave(this);}

    void raise_w();
    void maximize_w();
    void minimize_w();
    void close_w();
    void demaximize_w();

    // Deferred until workspace_manager has its initial workspace list.
    void populateDeskMenu();
    void moveToDesktop(int workspace);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    ForeignToplevelHandle *handle;
    ExtWorkspaceManager *workspace_manager;
    BiomeWorkspaces *biome_workspaces;
    QString m_appId;

    bool dragActive = false;
    bool allowReleaseAction = true;
    QPoint dragPos;

    QMenu *pmenu = nullptr;
    QMenu *desk_menu = nullptr; // pmenu's submenu, only with Biome workspaces
};

#endif // WINDOWBUTTON_H
