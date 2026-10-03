// SPDX-License-Identifier: LGPL-3.0-or-later

#include "panelanchor.h"
#include "menuanchor.h"

#include <QCursor>
#include <QSettings>
#include <QWidget>

PanelAnchor panelAnchor(QWidget *launcher, PositionpPolicy policy)
{
    QWidget *toplevel = launcher->window();
    bool top = QSettings("Forest","Panel").value("position").toString().toLower() == "top";

    // Full toplevel height so the edges below are the panel's, not the launcher's.
    PanelAnchor result;
    if (policy == CenteredOnMouse || policy == EdgeAlignedOnMouse)
        result.rect = QRect(toplevel->mapFromGlobal(QCursor::pos()).x(), 0, 1, toplevel->height());
    else
        result.rect = QRect(launcher->mapTo(toplevel, QPoint(0,0)).x(), 0, launcher->width(), toplevel->height());

    result.anchor = top ? Qt::Edges(Qt::BottomEdge) : Qt::Edges(Qt::TopEdge);
    result.gravity = result.anchor;
    if (policy == EdgeAlignedOnWidget || policy == EdgeAlignedOnMouse) {
        result.anchor |= Qt::LeftEdge;
        result.gravity |= Qt::RightEdge;
    }
    return result;
}

void anchorMenuOnLauncher(QMenu *menu, QWidget *launcher, PositionpPolicy policy)
{
    const PanelAnchor a = panelAnchor(launcher, policy);
    menuanchor::anchorMenu(menu, launcher->window(), a.rect, a.anchor, a.gravity,
                           menuanchor::SlideX | menuanchor::SlideY);
}
