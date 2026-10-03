// SPDX-License-Identifier: LGPL-3.0-or-later

#include "menuanchor.h"

#include <QEvent>
#include <QMenu>
#include <QStyle>
#include <QWindow>

namespace menuanchor {

namespace {

void setPositioner(QWindow *handle, QWindow *parent, QRect rect,
                   Qt::Edges anchorEdges, Qt::Edges gravity, uint constraints)
{
    // The anchor rect is relative to the transient parent's surface.
    handle->setTransientParent(parent);
    handle->setProperty("_q_waylandPopupAnchorRect", rect);
    handle->setProperty("_q_waylandPopupAnchor", QVariant::fromValue(anchorEdges));
    handle->setProperty("_q_waylandPopupGravity", QVariant::fromValue(gravity));
    handle->setProperty("_q_waylandPopupConstraintAdjustment", constraints);
}

QMenu *parentMenu(QMenu *submenu)
{
    const auto objects = submenu->menuAction()->associatedObjects();
    for (QObject *object : objects) {
        QMenu *menu = qobject_cast<QMenu*>(object);
        if (menu && menu != submenu && menu->isVisible())
            return menu;
    }
    return nullptr;
}

// The transparent QSS margin around the visible frame: the stylesheet style folds
// it into PM_MenuPanelWidth, while PM_DefaultFrameWidth is the border alone.
int menuMargin(QMenu *menu)
{
    menu->ensurePolished();
    QStyle *style = menu->style();
    return qMax(0, style->pixelMetric(QStyle::PM_MenuPanelWidth, nullptr, menu)
                   - style->pixelMetric(QStyle::PM_DefaultFrameWidth, nullptr, menu));
}

// Anchor/gravity match Qt >= 6.11's submenus, but Qt ignores the QSS margin, so keep
// this after upgrading. The rect is the parent's visible frame
// inset by the submenu's margin, so frames touch whichever side it flips to; first
// items align as in QMenu::internalDelayedPopup.
void anchorSubmenu(QMenu *submenu)
{
    QMenu *parent = parentMenu(submenu);
    if (!parent || !parent->windowHandle() || !submenu->windowHandle())
        return;
    const int inset = menuMargin(parent) + menuMargin(submenu);
    const QRect item = parent->actionGeometry(submenu->menuAction());
    const int firstItemTop = submenu->actions().isEmpty()
        ? 0 : submenu->actionGeometry(submenu->actions().first()).top();
    const QRect rect(inset, item.top() - firstItemTop,
                     qMax(1, parent->width() - 2 * inset), qMax(1, item.height()));
    setPositioner(submenu->windowHandle(), parent->windowHandle(), rect,
                  Qt::TopEdge | Qt::RightEdge, Qt::BottomEdge | Qt::RightEdge,
                  FlipX | SlideY);
}

}

void anchorMenu(QMenu *menu, QWidget *anchor, QRect rectInAnchor,
                Qt::Edges anchorEdges, Qt::Edges gravity, uint constraints)
{
    menu->ensurePolished(); // translucency must be set before winId()
    menu->winId();
    QWidget *toplevel = anchor->window();
    toplevel->winId();
    if (!menu->windowHandle() || !toplevel->windowHandle())
        return;
    QRect rect(anchor->mapTo(toplevel, rectInAnchor.topLeft()), rectInAnchor.size());
    setPositioner(menu->windowHandle(), toplevel->windowHandle(), rect, anchorEdges, gravity, constraints);
}

// A ±margin span rather than a point, so the visible corner (not the window's)
// lands on pos, including after a flip.
void anchorMenuAtPoint(QMenu *menu, QWidget *anchor, QPoint pos)
{
    const int m = menuMargin(menu);
    const int span = qMax(1, 2 * m);
    anchorMenu(menu, anchor, QRect(pos - QPoint(m, m), QSize(span, span)),
               Qt::TopEdge | Qt::LeftEdge, Qt::BottomEdge | Qt::RightEdge,
               FlipX | FlipY | SlideX | SlideY);
}

// Show arrives before the platform window is shown, early enough for the positioner.
// Polish is app-wide so it also catches menus Forest doesn't build (dbusmenu, QLineEdit).
bool MenuFilter::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() != QEvent::Polish && event->type() != QEvent::Show)
        return false;
    QMenu *menu = qobject_cast<QMenu*>(watched);
    if (!menu)
        return false;
    if (event->type() == QEvent::Polish)
        menu->setAttribute(Qt::WA_TranslucentBackground);
    else
        anchorSubmenu(menu);
    return false;
}

}
