// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WINDOWBUTTON_H
#define WINDOWBUTTON_H

#include "panelbutton.h"
#include "popupmenu.h"
#include "foreigntoplevelhandle.h"

class windowbutton : public panelbutton{
    Q_OBJECT

public:
    explicit windowbutton(ForeignToplevelHandle *handle, QIcon icon, QString text);

    ForeignToplevelHandle *toplevelHandle(){return handle;}

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

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    ForeignToplevelHandle *handle;

    bool dragActive = false;
    bool allowReleaseAction = true;
    QPoint dragPos;

    popupmenu *pmenu = nullptr;
    // "Move to desktop" has no wlr-foreign-toplevel-management backing -
    // no protocol here exposes a workspace concept at all. Kept, disabled,
    // as a reminder to re-wire this once Workstream D (deskswitch) lands a
    // workspace protocol. See biome/docs/phase4-plan.md Workstream B/D.
    popupmenu *desk_menu = nullptr;
};

#endif // WINDOWBUTTON_H
