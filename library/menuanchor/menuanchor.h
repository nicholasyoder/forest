// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MENUANCHOR_H
#define MENUANCHOR_H

#include <QObject>
#include <QRect>

class QMenu;
class QWidget;

namespace menuanchor {

// xdg_positioner.constraint_adjustment values (QtWayland's enum is private).
enum ConstraintAdjustment : uint { SlideX = 1, SlideY = 2, FlipX = 4, FlipY = 8 };

// Places a menu via xdg_positioner relative to anchor's window, since Qt can't
// compute global positions for layer surfaces. Call before every popup()/exec().
void anchorMenu(QMenu *menu, QWidget *anchor, QRect rectInAnchor,
                Qt::Edges anchorEdges, Qt::Edges gravity, uint constraints);

// Context menu at a point in anchor's coordinates; flips away from screen edges.
void anchorMenuAtPoint(QMenu *menu, QWidget *anchor, QPoint pos);

// App-wide filter: translucency for rounded QSS menus, and submenu anchoring.
class MenuFilter : public QObject
{
public:
    using QObject::QObject;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};

}

#endif // MENUANCHOR_H
