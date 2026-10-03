// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PANELANCHOR_H
#define PANELANCHOR_H

#include <QRect>

class QMenu;
class QWidget;

enum PositionpPolicy { CenteredOnWidget, EdgeAlignedOnWidget, CenteredOnMouse, EdgeAlignedOnMouse };

// xdg_positioner anchor for a popup off a panel launcher; rect is in launcher->window() coords.
struct PanelAnchor {
    QRect rect;
    Qt::Edges anchor;
    Qt::Edges gravity;
};

PanelAnchor panelAnchor(QWidget *launcher, PositionpPolicy policy);

// Call before every popup()/exec() of a menu opened from a panel launcher.
void anchorMenuOnLauncher(QMenu *menu, QWidget *launcher, PositionpPolicy policy);

// Anchors and pops up menu, unless this click on launcher is the one that just closed it.
// Keep menu unparented: panelbutton's own stylesheet would cascade into it.
void popupMenuOnLauncher(QMenu *menu, QWidget *launcher, PositionpPolicy policy);

// True while delivering the release whose press closed menu, if menu was last opened from launcher.
// For async openers (the tray) that must check before popupMenuOnLauncher runs.
bool menuClosedByLauncherClick(QMenu *menu, QWidget *launcher);

#endif // PANELANCHOR_H
