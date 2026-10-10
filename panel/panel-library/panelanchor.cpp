// SPDX-License-Identifier: LGPL-3.0-or-later

#include "panelanchor.h"
#include "panelconfig.h"
#include "menuanchor.h"

#include <QApplication>
#include <QCursor>
#include <QMenu>
#include <QPointer>
#include <QSettings>
#include <QTimer>
#include <QWidget>

namespace {

// Menus open on right release, so a right-click on the open menu's launcher would
// close it on press and reopen it on release. Flags that press until its release is delivered.
class ToggleGuard : public QObject
{
public:
    explicit ToggleGuard(QMenu *menu) : QObject(menu)
    {
        connect(menu, &QMenu::aboutToHide, this, [this]{
            if (QGuiApplication::mouseButtons() & Qt::RightButton) {
                closedByPress = true;
                qApp->installEventFilter(this);
            }
        });
    }

    QPointer<QWidget> launcher;
    bool closedByPress = false;

protected:
    bool eventFilter(QObject *, QEvent *event) override
    {
        if (event->type() == QEvent::MouseButtonRelease) {
            qApp->removeEventFilter(this);
            QTimer::singleShot(0, this, [this]{ closedByPress = false; });
        }
        return false;
    }
};

ToggleGuard *toggleGuard(QMenu *menu)
{
    for (QObject *child : menu->children())
        if (auto *guard = dynamic_cast<ToggleGuard*>(child))
            return guard;
    return nullptr;
}

}

PanelAnchor panelAnchor(QWidget *launcher, PositionpPolicy policy)
{
    QWidget *toplevel = launcher->window();
    bool top = QSettings("Forest","Panel").value(panelconfig::position, panelconfig::position_default).toString().toLower() == "top";

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

bool menuClosedByLauncherClick(QMenu *menu, QWidget *launcher)
{
    const ToggleGuard *guard = toggleGuard(menu);
    return guard && guard->closedByPress && guard->launcher == launcher;
}

void popupMenuOnLauncher(QMenu *menu, QWidget *launcher, PositionpPolicy policy)
{
    if (menuClosedByLauncherClick(menu, launcher))
        return;
    ToggleGuard *guard = toggleGuard(menu);
    if (!guard)
        guard = new ToggleGuard(menu);
    guard->launcher = launcher;
    anchorMenuOnLauncher(menu, launcher, policy);
    menu->popup(launcher->mapToGlobal(QPoint(0, 0)));
}
