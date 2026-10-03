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
#include <QPointer>
#include <QTimer>

#include "panelanchor.h"

class popup : public QWidget
{
    Q_OBJECT
public:
    popup(QLayout *layout, QWidget *launcherw, PositionpPolicy policy){
        contentlayout = layout;
        launcherwidget = launcherw;
        pospolicy = policy;

        // Not Qt::Popup: that needs an xdg_popup grab, which fails without a recent
        // input serial (e.g. hotkey-triggered), degrading to a decorated toplevel.
        // Cost: click-outside-closes is hand-rolled in eventFilter()/event().
        setWindowFlags(Qt::ToolTip);
        setAttribute(Qt::WA_TranslucentBackground);

        QHBoxLayout *hlayout = new QHBoxLayout(this);
        hlayout->setContentsMargins(QMargins(0,0,0,0));
        popupQFrame = new QFrame;
        popupQFrame->setObjectName("popup");
        popupQFrame->setLayout(contentlayout);
        hlayout->addWidget(popupQFrame);

        connect(this, &popup::outsideclicked, this, &popup::closepopup);
        pendingTimeout.setSingleShot(true);
        connect(&pendingTimeout, &QTimer::timeout, this, &popup::finishShow);
    }

    QFrame *popupQFrame = nullptr;

signals:
    void keypressed(QKeyEvent *event);
    void mousereleased(QMouseEvent *event);
    void outsideclicked();

public slots:
    void showpopup(){
        if (!launcherwidget) return;
        QWidget *toplevel = launcherwidget->window();
        toplevel->winId();
        QWindow *toplevelHandle = toplevel->windowHandle();

        // LayerShellQt can't attach a popup to a layer surface that hasn't been
        // configured yet (first hotkey right after startup) - wait for its expose.
        // Capped: QtWayland also reports unexposed after any frame-callback timeout.
        if (toplevelHandle && !toplevelHandle->isExposed()) {
            if (pendingParent != toplevelHandle) {
                cancelPendingShow();
                pendingParent = toplevelHandle;
                toplevelHandle->installEventFilter(this);
                pendingTimeout.start(500);
            }
            return;
        }
        finishShow();
    }

    void closepopup(){
        cancelPendingShow();
        this->close();
    }

    void focuschild(bool next = true)
    {
        if (next)
            this->focusNextChild();
        else
            this->focusPreviousChild();
    }

    void changelauncher(QWidget *launcher){launcherwidget = launcher;}

protected:
    void keyPressEvent(QKeyEvent *event){emit keypressed(event);}//so the object controlling the popup can use keystokes
    void mouseReleaseEvent(QMouseEvent *event){emit mousereleased(event);}//and mouse clicks

    void showEvent(QShowEvent *e) override {
        qApp->installEventFilter(this);
        QWidget::showEvent(e);
    }

    void hideEvent(QHideEvent *e) override {
        qApp->removeEventFilter(this);
        QWidget::hideEvent(e);
    }

    // In-process click-outside. Launcher clicks are skipped so its own toggle
    // handler wins; lastPressOnLauncher also suppresses the WindowDeactivate
    // that press causes (cleared next turn so it can't go stale).
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (watched == pendingParent && event->type() == QEvent::Expose && pendingParent->isExposed()) {
            pendingTimeout.start(0);
        }
        else if (event->type() == QEvent::MouseButtonPress && isVisible()) {
            QWidget *clicked = qobject_cast<QWidget*>(watched);
            bool onLauncher = clicked && launcherwidget
                && (clicked == launcherwidget || launcherwidget->isAncestorOf(clicked));
            if (onLauncher) {
                lastPressOnLauncher = true;
                QTimer::singleShot(0, this, [this]{ lastPressOnLauncher = false; });
            } else if (clicked && clicked != this && !this->isAncestorOf(clicked)) {
                emit outsideclicked();
            }
        }
        return QWidget::eventFilter(watched, event);
    }

    // Cross-process click-outside: Biome gives this window real focus despite no grab.
    bool event(QEvent *e) override {
        if (e->type() == QEvent::WindowDeactivate && isVisible()) {
            bool skip = lastPressOnLauncher;
            lastPressOnLauncher = false;
            if (!skip) closepopup();
        }
        return QWidget::event(e);
    }

private:
    // Placed via xdg_positioner (anchor rect relative to the parent toplevel) since
    // layer surfaces have no known global position. Private QtWayland properties,
    // same as LayerShellQt/Plasma use.
    void positionOnLauncher(){
        winId();
        QWindow *handle = windowHandle();
        if (!handle) return;

        QWidget *toplevel = launcherwidget->window();
        // Explicit parent: QtWayland otherwise guesses from the last input window.
        handle->setTransientParent(toplevel->windowHandle());

        const PanelAnchor anchor = panelAnchor(launcherwidget, pospolicy);
        handle->setProperty("_q_waylandPopupAnchorRect", anchor.rect);
        handle->setProperty("_q_waylandPopupAnchor", QVariant::fromValue(anchor.anchor));
        handle->setProperty("_q_waylandPopupGravity", QVariant::fromValue(anchor.gravity));
    }

    void finishShow(){
        cancelPendingShow();
        if (!launcherwidget) return;
        adjustSize(); // fit current content; a hidden-time resize() would otherwise stick
        positionOnLauncher();
        show();
    }

    void cancelPendingShow(){
        pendingTimeout.stop();
        if (pendingParent) pendingParent->removeEventFilter(this);
        pendingParent = nullptr;
    }

    QLayout *contentlayout;
    QPointer<QWidget> launcherwidget;
    QPointer<QWindow> pendingParent;
    QTimer pendingTimeout;
    PositionpPolicy pospolicy;
    bool lastPressOnLauncher = false;
};

#endif // POPUPBOX_H
