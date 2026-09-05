// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef POPUPBOX_H
#define POPUPBOX_H

#include <QWidget>
#include <QSettings>
#include <QDebug>
#include <QCloseEvent>
#include <QApplication>
#include <QFrame>
#include <QVBoxLayout>
#include <QWindow>
#include <QDeadlineTimer>
#include <QEventLoop>

enum PositionpPolicy { CenteredOnWidget, EdgeAlignedOnWidget, CenteredOnMouse, EdgeAlignedOnMouse };

class popup : public QWidget
{
    Q_OBJECT
public:
    popup(QLayout *layout, QWidget *launcherw, PositionpPolicy policy){
        contentlayout = layout;
        launcherwidget = launcherw;
        pospolicy = policy;

        // Qt::Popup implicitly requests a Wayland grab, which requires a
        // real, recent input serial - something a popup shown without
        // genuine antecedent input (e.g. triggered over D-Bus by a global
        // hotkey rather than a click) never has, so QtWaylandClient falls
        // back to mapping it as a plain decorated toplevel instead of a real
        // xdg_popup. Qt::ToolTip gets the same "no decoration, no WM
        // management, doesn't take focus" treatment without requiring a
        // grab, so it works identically whether triggered by click, hover,
        // or hotkey/D-Bus. This is a real, currently-unsolved-in-the-wild
        // limitation of xdg_popup's grab model, not something special to
        // Forest - KDE Plasma hits the identical failure for Kickoff's own
        // global-shortcut-triggered menu on Wayland/KWin, unfixed as of
        // this writing. A compositor-side fix (Biome granting a legitimate
        // input serial when it dispatches a GlobalShortcuts signal) was
        // considered and not attempted, on the evidence that KWin - despite
        // integrating shortcut dispatch directly into the compositor
        // process specifically to be able to see raw input - hasn't solved
        // it either. Cost of Qt::ToolTip: the automatic "click outside
        // closes it" a real grab provided for free is gone, so it's
        // hand-rolled below via an application-wide event
        // filter instead.
        setWindowFlags(Qt::ToolTip);
        setAttribute(Qt::WA_TranslucentBackground);

        QHBoxLayout *hlayout = new QHBoxLayout(this);
        hlayout->setContentsMargins(QMargins(0,0,0,0));
        popupQFrame = new QFrame;
        popupQFrame->setObjectName("popup");
        popupQFrame->setLayout(contentlayout);
        hlayout->addWidget(popupQFrame);

        qApp->installEventFilter(this);
        connect(this, &popup::outsideclicked, this, &popup::closepopup);
    }

    ~popup(){
        qApp->removeEventFilter(this);
    }

    QFrame *popupQFrame = nullptr;

signals:
    void keypressed(QKeyEvent *event);
    void mousereleased(QMouseEvent *event);
    void outsideclicked();

public slots:
    void showpopup(){
        positionOnLauncher();
        show();
    }

    void closepopup(){this->close();}

    void focuschild(bool next = true)
    {
        if (next)
            this->focusNextChild();
        else
            this->focusPreviousChild();
    }

    void changelauncher(QWidget *launcher){launcherwidget = launcher;}

    // Popups are placed via Wayland's protocol-level positioner (an anchor
    // rect + edge + gravity, all relative to the parent surface) rather than
    // an absolute desktop position - layer-shell's configure event only ever
    // reports size, never position, so there's no reliable global coordinate
    // a client could place a popup with. This expresses the popup's
    // placement in coordinates local to the panel (real, reliably-known
    // values - no cross-window global tracking involved), matching what the
    // xdg_positioner protocol actually expects. Set through the same
    // private QtWayland properties LayerShellQt/KDE Plasma itself relies on
    // for this.
    void positionOnLauncher(){
        QWidget *panel = getpanelwidget();
        if (!panel) return;

        winId(); // force native window creation so windowHandle() is valid
        QWindow *handle = windowHandle();
        if (!handle) return;

        // Without an explicit transient parent, QtWaylandClient can only
        // guess one from whichever window most recently received real
        // pointer/keyboard input - which is unset (or stale, pointing
        // somewhere else entirely) for a popup shown without a genuine
        // antecedent input event, e.g. triggered over D-Bus by a global
        // hotkey rather than a click. Without a resolvable parent, it falls
        // back to mapping this window as a plain decorated toplevel instead
        // of a popup - the actual cause of a Biome-drawn border showing up
        // on hotkey-triggered popups. Setting it explicitly (the same thing
        // QMenu/QToolTip do internally) makes parent resolution reliable
        // regardless of input history.
        QWidget *toplevel = launcherwidget->window();
        toplevel->winId();
        QWindow *toplevelHandle = toplevel->windowHandle();

        // On the very first popup shown right after Forest starts (e.g. a
        // hotkey pressed the instant the process comes up, before any real
        // click has pumped the Wayland event loop), the panel's layer-shell
        // surface can still be mid-setup: LayerShellQt hasn't yet gotten the
        // compositor's first zwlr_layer_surface_v1.configure ack, so it
        // doesn't yet recognize this toplevel as a layer surface at all.
        // LayerShellQt's own popup-attachment code
        // (QWaylandLayerSurface::attachPopup(), invent.kde.org/plasma/
        // layer-shell-qt) then logs "Cannot attach popup of unknown type"
        // and this falls back to a plain, bordered toplevel - confirmed via
        // the exact log line and matching source (std::any_cast<xdg_popup*>
        // on the popup's own surfaceRole() failing because Qt's xdg-shell
        // side hasn't finished creating that role object yet either, since
        // it's gated on the same parent-readiness). Any later popup works
        // fine because by then real input has already forced this setup to
        // complete once. Wait for the same readiness synchronously here
        // instead of depending on that having happened by chance.
        if (toplevelHandle && !toplevelHandle->isExposed()) {
            QDeadlineTimer deadline(500);
            while (!toplevelHandle->isExposed() && !deadline.hasExpired())
                QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents, 50);
        }

        if (toplevelHandle)
            handle->setTransientParent(toplevelHandle);

        QString panelpos = psettings->value("position").toString().toLower();
        bool top = (panelpos == "top");

        // Span the anchor rect over the panel's full height (not the
        // launcher's own) so TopEdge/BottomEdge below refer to the panel's
        // true edge - a launcher widget shorter than the panel row is
        // vertically centered within it by the layout, so anchoring to the
        // launcher's own top/bottom would land a few pixels short of the
        // panel's actual edge and let the popup overlap it.
        int launcherLocalX = launcherwidget->mapTo(panel, QPoint(0,0)).x();
        QRect anchorRect(launcherLocalX, 0, launcherwidget->width(), panel->height());

        Qt::Edges edge = top ? Qt::Edges(Qt::BottomEdge) : Qt::Edges(Qt::TopEdge);
        Qt::Edges gravity = edge;
        if (pospolicy == EdgeAlignedOnWidget || pospolicy == EdgeAlignedOnMouse) {
            edge |= Qt::LeftEdge;
            gravity |= Qt::RightEdge;
        }

        handle->setProperty("_q_waylandPopupAnchorRect", anchorRect);
        handle->setProperty("_q_waylandPopupAnchor", QVariant::fromValue(edge));
        handle->setProperty("_q_waylandPopupGravity", QVariant::fromValue(gravity));
    }

protected:
    void keyPressEvent(QKeyEvent *event){emit keypressed(event);}//so the object controlling the popup can use keystokes
    void mouseReleaseEvent(QMouseEvent *event){emit mousereleased(event);}//and mouse clicks

    // Restores the "click outside closes it" behavior Qt::Popup's grab used
    // to provide for free, now that this is Qt::ToolTip and has none. Only
    // catches presses Forest's own process actually receives - a click
    // straight onto a genuinely different application's window still won't
    // deliver anything to us without a grab, so this can't close the popup
    // in that case. launcherwidget is excluded so a click on the button that
    // toggles this popup open/closed falls through to that button's own
    // click handler instead of racing it closed here first (which would
    // otherwise make the button's own "already open, so close" branch fire
    // on an already-closed popup and reopen it).
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (event->type() == QEvent::MouseButtonPress && isVisible()) {
            QWidget *clicked = qobject_cast<QWidget*>(watched);
            if (clicked && clicked != this && !this->isAncestorOf(clicked)
                && (!launcherwidget || (clicked != launcherwidget && !launcherwidget->isAncestorOf(clicked))))
                emit outsideclicked();
        }
        return QWidget::eventFilter(watched, event);
    }

private slots:
    QWidget* getpanelwidget() {
        QWidget *parent = launcherwidget->parentWidget();
        while (1) {
            if (parent){
                if (parent->objectName() == "panel") return parent;
                else parent = parent->parentWidget();
            }
            else return nullptr;
        }
    }

private:
    //bool allowclose = false;
    QLayout *contentlayout;
    QWidget *launcherwidget;
    PositionpPolicy pospolicy;
    QSettings *psettings = new QSettings("Forest","Panel");
};

#endif // POPUPBOX_H
