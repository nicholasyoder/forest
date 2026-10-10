// SPDX-License-Identifier: LGPL-3.0-or-later

#include "xdgactivation.h"

#include <QAction>
#include <QDebug>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QProcess>
#include <QTimer>
#include <QWindow>
#include <qguiapplication_platform.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

namespace {
// Per-instance: panel and desktop are separate modules in one process, each with its own copy.
constexpr char kWatchedProperty[] = "_forest_xdg_activation";
}

class XdgActivationToken : public QObject, public QtWayland::xdg_activation_token_v1 {
    Q_OBJECT

public:
    explicit XdgActivationToken(struct ::xdg_activation_token_v1 *object)
        : QtWayland::xdg_activation_token_v1(object) {}
    ~XdgActivationToken() override { destroy(); }

    QString token;
    bool received = false;

signals:
    void done();

protected:
    void xdg_activation_token_v1_done(const QString &value) override {
        token = value;
        received = true;
        emit done();
    }
};

XdgActivation *XdgActivation::instance() {
    // Leaked on purpose: Wayland objects must not outlive the display at exit.
    static XdgActivation *s_instance = new XdgActivation;
    return s_instance;
}

XdgActivation::XdgActivation() : QWaylandClientExtensionTemplate<XdgActivation>(1) {
    qApp->installEventFilter(this);
}

void XdgActivation::watch(QAction *action) {
    action->setProperty(kWatchedProperty, QVariant::fromValue<void*>(this));
}

bool XdgActivation::eventFilter(QObject *watched, QEvent *event) {
    QAction *action = nullptr;
    QMenu *menu = qobject_cast<QMenu*>(watched);
    if (menu && event->type() == QEvent::MouseButtonRelease) {
        action = menu->actionAt(static_cast<QMouseEvent*>(event)->position().toPoint());
    }
    else if (menu && event->type() == QEvent::KeyPress) {
        int key = static_cast<QKeyEvent*>(event)->key();
        if (key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Space)
            action = menu->activeAction();
    }
    if (action && action->property(kWatchedProperty).value<void*>() == this)
        request(menu);
    return false;
}

void XdgActivation::request(QWidget *source) {
    auto *app = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    QWindow *window = source->window()->windowHandle();
    auto *waylandWindow = window ? dynamic_cast<QtWaylandClient::QWaylandWindow*>(window->handle()) : nullptr;
    if (!isActive() || !app || !waylandWindow || !waylandWindow->wlSurface())
        return;

    delete pending;
    pending = new XdgActivationToken(get_activation_token());
    pending->set_serial(app->lastInputSerial(), app->lastInputSeat());
    pending->set_surface(waylandWindow->wlSurface());
    pending->commit();
    // `triggered` runs inside this same event; anything left after it was not a launch.
    QTimer::singleShot(0, pending.data(), [this, token = pending.data()](){
        if (pending == token)
            token->deleteLater();
    });
}

void XdgActivation::launch(const QString &program, const QStringList &arguments) {
    launch([program, arguments](){
        if (!QProcess::startDetached(program, arguments))
            qWarning() << "Failed to launch" << program;
    });
}

void XdgActivation::launch(const std::function<void()> &start) {
    auto run = [start](const QString &token){
        // Unset otherwise, so our own launch token isn't passed on stale.
        if (token.isEmpty())
            qunsetenv("XDG_ACTIVATION_TOKEN");
        else
            qputenv("XDG_ACTIVATION_TOKEN", token.toUtf8());
        start();
        qunsetenv("XDG_ACTIVATION_TOKEN");
    };

    XdgActivationToken *token = pending;
    pending = nullptr;
    if (!token) {
        run(QString());
        return;
    }
    if (token->received) {
        run(token->token);
        token->deleteLater();
        return;
    }
    connect(token, &XdgActivationToken::done, token, [token, run](){
        run(token->token);
        token->deleteLater();
    });
}

#include "xdgactivation.moc"
