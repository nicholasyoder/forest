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

#endif // PANELANCHOR_H
