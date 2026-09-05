// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WINDOWBUTTON_H
#define WINDOWBUTTON_H

#include "panelbutton.h"
#include "popupmenu.h"
#include "foreigntoplevelhandle.h"
#include "extworkspacemanager.h"

class windowbutton : public panelbutton{
    Q_OBJECT

public:
    explicit windowbutton(ForeignToplevelHandle *handle, QIcon icon, QString text, ExtWorkspaceManager *workspaceManager);

    ForeignToplevelHandle *toplevelHandle(){return handle;}

    // Set once ext-foreign-toplevel-list's identifier arrives for this
    // window - see windowlist.cpp's pairing queue (tryPairPendingHandles()).
    // Empty until then, which only briefly matters: moveToDesktop() reads
    // this lazily at click time (by which point pairing has essentially
    // always completed), and windowlist's per-desktop visibility filter
    // treats an unpaired button as visible until it can be classified.
    void setIdentifier(const QString &id){m_identifier = id;}
    QString identifier() const{return m_identifier;}

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

    // Builds one "move to this desktop" item per workspace into desk_menu -
    // deferred until workspace_manager has its initial workspace list
    // (org.biome.Workspaces/ext-workspace-v1 have no other way to enumerate
    // them). Biome's workspace set is fixed at startup, so this only ever
    // runs once in practice.
    void populateDeskMenu();
    void moveToDesktop(int workspace);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    ForeignToplevelHandle *handle;
    ExtWorkspaceManager *workspace_manager;
    QString m_identifier;

    bool dragActive = false;
    bool allowReleaseAction = true;
    QPoint dragPos;

    popupmenu *pmenu = nullptr;
    // "Move to Desktop" submenu, one item per workspace - see
    // populateDeskMenu().
    popupmenu *desk_menu = nullptr;
};

#endif // WINDOWBUTTON_H
