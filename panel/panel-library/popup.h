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
#include <utility>
#include <QtGui/qguiapplication_platform.h>

#include "panelanchor.h"
#include "xdgactivation.h"

class popup : public QWidget
{
    Q_OBJECT
public:
    // Click-opened popups grab. Hotkey-opened ones can't (no press serial), so they get
    // focus from an xdg-activation token instead; non-grabbing popups (those, and hover
    // thumbnails) hand-roll click-outside in eventFilter()/event().
    popup(QLayout *layout, QWidget *launcherw, PositionpPolicy policy, bool grabbing = true){
        contentlayout = layout;
        launcherwidget = launcherw;
        pospolicy = policy;
        this->grabbing = grabbing;

        setWindowFlags(Qt::ToolTip); // finishShow() picks the real type
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
    void showpopup(){ showpopupActivated(QString()); }

    // activationToken: from a hotkey (GlobalShortcuts' activation_token), for keyboard focus.
    void showpopupActivated(const QString &activationToken){
        if (!launcherwidget || closedByPress) return;
        pendingToken = activationToken;
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

    // Qt delivers the press that closed us to the launcher too; keep the filter until
    // its release so that click toggles instead of reopening.
    void hideEvent(QHideEvent *e) override {
        if (QGuiApplication::mouseButtons() != Qt::NoButton)
            closedByPress = true;
        else
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
        else if (closedByPress && !isVisible() && event->type() == QEvent::MouseButtonRelease) {
            qApp->removeEventFilter(this);
            QTimer::singleShot(0, this, [this]{ closedByPress = false; });
        }
        else if (!isGrabbing() && event->type() == QEvent::MouseButtonPress && isVisible()) {
            QWidget *clicked = qobject_cast<QWidget*>(watched);
            bool onLauncher = clicked && launcherwidget
                && (clicked == launcherwidget || launcherwidget->isAncestorOf(clicked));
            if (onLauncher) {
                lastPressOnLauncher = true;
                QTimer::singleShot(0, this, [this]{ lastPressOnLauncher = false; });
            } else if (clicked && clicked != this && !this->isAncestorOf(clicked)
                       && !isChainedPopup(clicked->window()->windowHandle())) {
                emit outsideclicked();
            }
        }
        return QWidget::eventFilter(watched, event);
    }

    // Cross-process click-outside: Biome gives this window real focus despite no grab.
    // A chained grabbing popup (e.g. a context menu) takes focus without closing this.
    bool event(QEvent *e) override {
        if (!isGrabbing() && e->type() == QEvent::WindowDeactivate && isVisible()) {
            bool skip = lastPressOnLauncher || chainedPopupVisible();
            lastPressOnLauncher = false;
            if (!skip) closepopup();
        }
        return QWidget::event(e);
    }

private:
    bool isGrabbing() const { return windowType() == Qt::Popup; }

    // Qt 6.8 makes a Qt::Popup a toplevel until the app has seen a press (later Qts crash).
    static bool hasInputSeat(){
        auto *wayland = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
        return wayland && wayland->lastInputSeat();
    }

    // A popup window transient (directly or not) to this one.
    bool isChainedPopup(QWindow *window) const {
        QWindow *self = windowHandle();
        if (!window || window == self) return false;
        for (window = window->transientParent(); window; window = window->transientParent())
            if (window == self) return true;
        return false;
    }

    bool chainedPopupVisible() const {
        const auto windows = QGuiApplication::topLevelWindows();
        for (QWindow *window : windows)
            if (window->isVisible() && isChainedPopup(window)) return true;
        return false;
    }

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
        const QString token = std::exchange(pendingToken, QString());
        const Qt::WindowType type = grabbing && token.isEmpty() && hasInputSeat() ? Qt::Popup : Qt::ToolTip;
        if (windowType() != type)
            setWindowFlags(type);
        adjustSize(); // fit current content; a hidden-time resize() would otherwise stick
        positionOnLauncher();
        show();
        if (!token.isEmpty())
            XdgActivation::instance()->activateWindow(this, token);
    }

    void cancelPendingShow(){
        pendingTimeout.stop();
        if (pendingParent) pendingParent->removeEventFilter(this);
        pendingParent = nullptr;
    }

    QLayout *contentlayout;
    QPointer<QWidget> launcherwidget;
    QPointer<QWindow> pendingParent;
    QString pendingToken;
    QTimer pendingTimeout;
    PositionpPolicy pospolicy;
    bool lastPressOnLauncher = false;
    bool grabbing;
    bool closedByPress = false;
};

#endif // POPUPBOX_H
