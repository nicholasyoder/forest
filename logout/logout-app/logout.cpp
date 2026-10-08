// SPDX-License-Identifier: LGPL-3.0-or-later

#include "logout.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScreen>
#include <QWindow>
#include <QMessageBox>
#include <QVector>

#include <LayerShellQt/Window>

#include "miscutills.h"

struct ActionData {
    QString key;
    QString method; // org.freedesktop.login1 method
};

const QMap<ActionType, ActionData> action_map = {
    {ActionType::LOCK, {"lock", "Lock"}},
    {ActionType::SHUTDOWN, {"shutdown", "PowerOff"}},
    {ActionType::REBOOT, {"reboot", "Reboot"}},
    {ActionType::LOGOUT, {"logout", "Terminate"}},
    {ActionType::SUSPEND, {"suspend", "Suspend"}},
    {ActionType::HIBERNATE, {"hibernate", "Hibernate"}}
};

const QString LOGIN1_SERVICE = "org.freedesktop.login1";
const QString LOGIN1_PATH = "/org/freedesktop/login1";
const QString LOGIN1_MANAGER = "org.freedesktop.login1.Manager";

// Returns an invalid QDBusError on success.
QDBusError call_login1(const QString &method){
    QDBusMessage msg;
    if (method == "Terminate" || method == "Lock") // the per-session self object avoids resolving our session id
        msg = QDBusMessage::createMethodCall(LOGIN1_SERVICE, LOGIN1_PATH + "/session/self", "org.freedesktop.login1.Session", method);
    else // Manager methods take an "interactive" (polkit auth) bool
        msg = QDBusMessage::createMethodCall(LOGIN1_SERVICE, LOGIN1_PATH, LOGIN1_MANAGER, method) << true;
    return QDBusError(QDBusConnection::systemBus().call(msg));
}

bool login1_can(const QString &method){
    QDBusMessage reply = QDBusConnection::systemBus().call(
        QDBusMessage::createMethodCall(LOGIN1_SERVICE, LOGIN1_PATH, LOGIN1_MANAGER, "Can" + method));
    const QString answer = reply.arguments().value(0).toString();
    return answer == "yes" || answer == "challenge";
}

logoutmanager::logoutmanager(){
    setWindowFlags(Qt::FramelessWindowHint);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_TranslucentBackground);
    settings = new QSettings("Forest", "Logout");
    setup();

    winId(); // force native window creation so windowHandle() is valid
    LayerShellQt::Window *layer_window = LayerShellQt::Window::get(windowHandle());
    layer_window->setLayer(LayerShellQt::Window::LayerOverlay);
    layer_window->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);
    layer_window->setScope("forest-logout");

    // No anchors: layer-shell centres the surface. LayerShellQt defaults to all four.
    layer_window->setAnchors(LayerShellQt::Window::Anchors());
    if (QScreen *primary = ScreenTracker::primary())
        windowHandle()->setScreen(primary);

    overlays = layeroverlay::showOnAllScreens(QColor(0, 0, 0, 128), LayerShellQt::Window::LayerTop, "forest-logout-dim");
}

logoutmanager::~logoutmanager(){}

void logoutmanager::setup(){
    QVBoxLayout *basevlayout = new QVBoxLayout;
    basevlayout->setSpacing(0);
    basevlayout->setContentsMargins(QMargins(0,0,0,0));
    QHBoxLayout *closehlayout = new QHBoxLayout;
    closehlayout->setContentsMargins(QMargins(0,0,0,0));
    closehlayout->setSpacing(0);
    QLabel *titlelabel = new QLabel("Choose Action:");
    titlelabel->setObjectName("logout_TitleLabel");
    closehlayout->addWidget(titlelabel);
    closehlayout->addStretch(5);
    iconbutton *closebt = new iconbutton(QIcon::fromTheme("dialog-close"), 16, 16);
    closebt->setObjectName("logout_CloseButton");
    connect(closebt, SIGNAL(clicked()), this, SLOT(cancel()));
    closehlayout->addWidget(closebt);
    basevlayout->addLayout(closehlayout);
    QHBoxLayout *actionshlayout = new QHBoxLayout;
    actionshlayout->setSpacing(0);
    iconbutton *lockbt = new iconbutton(QIcon::fromTheme("system-lock-screen"), 48, 48, true, "Lock");
    actionshlayout->addWidget(lockbt);
    iconbutton *shutdownbt = new iconbutton(QIcon::fromTheme("system-shutdown"), 48, 48, true, "Shutdown");
    actionshlayout->addWidget(shutdownbt);
    iconbutton *rebootbt = new iconbutton(QIcon::fromTheme("system-reboot"), 48, 48, true, "Reboot");
    actionshlayout->addWidget(rebootbt);
    iconbutton *logoutbt = new iconbutton(QIcon::fromTheme("system-log-out"), 48, 48, true, "Logout");
    actionshlayout->addWidget(logoutbt);
    iconbutton *suspendbt = new iconbutton(QIcon::fromTheme("system-suspend"), 48, 48, true, "Suspend");
    actionshlayout->addWidget(suspendbt);
    iconbutton *hibernatebt = new iconbutton(QIcon::fromTheme("system-suspend-hibernate"), 48, 48, true, "Hibernate");
    actionshlayout->addWidget(hibernatebt);
    basevlayout->addLayout(actionshlayout);
    QFrame *mainframe = new QFrame;
    mainframe->setObjectName("logout_MainWindow");
    mainframe->setLayout(basevlayout);
    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setContentsMargins(QMargins(0,0,0,0));
    vlayout->addWidget(mainframe);

    QString lastaction = settings->value("lastaction", "shutdown").toString();
    if (lastaction == "lock") focusbt = lockbt;
    else if (lastaction == "shutdown") focusbt = shutdownbt;
    else if (lastaction == "reboot") focusbt = rebootbt;
    else if (lastaction == "logout") focusbt = logoutbt;
    else if (lastaction == "suspend") focusbt = suspendbt;
    else if (lastaction == "hibernate") focusbt = hibernatebt;

    // e.g. hibernate is "na" under Secure Boot lockdown
    const QList<std::pair<iconbutton*, QString>> gated = {{shutdownbt, "PowerOff"}, {rebootbt, "Reboot"},
                                                          {suspendbt, "Suspend"}, {hibernatebt, "Hibernate"}};
    for (const auto &[bt, method] : gated)
        bt->setEnabled(login1_can(method));
    if (!focusbt->isEnabled()) focusbt = logoutbt;

    connect(lockbt, &iconbutton::clicked, this, [this](){start_action(ActionType::LOCK);});
    connect(shutdownbt, &iconbutton::clicked, this, [this](){start_action(ActionType::SHUTDOWN);});
    connect(rebootbt, &iconbutton::clicked, this, [this](){start_action(ActionType::REBOOT);});
    connect(logoutbt, &iconbutton::clicked, this, [this](){start_action(ActionType::LOGOUT);});
    connect(suspendbt, &iconbutton::clicked, this, [this](){start_action(ActionType::SUSPEND);});
    connect(hibernatebt, &iconbutton::clicked, this, [this](){start_action(ActionType::HIBERNATE);});
    QWidget::setTabOrder(lockbt, shutdownbt);
    QWidget::setTabOrder(shutdownbt, rebootbt);
    QWidget::setTabOrder(rebootbt, logoutbt);
    QWidget::setTabOrder(logoutbt, suspendbt);
    QWidget::setTabOrder(suspendbt, hibernatebt);
    QWidget::setTabOrder(hibernatebt, closebt);
}

void logoutmanager::set_initial_focus(){
    activateWindow();
    focusbt->setFocus();
}

void logoutmanager::keyPressEvent(QKeyEvent *event){
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Up)
        this->focusPreviousChild();
    else if (event->key() == Qt::Key_Right || event->key() == Qt::Key_Down)
        this->focusNextChild();
    else if (event->key() == Qt::Key_Escape)
        cancel();
}

void logoutmanager::start_action(ActionType action){
    close(); // the compositor fades layer surfaces in/out by namespace

    const bool session_survives = action == ActionType::LOCK || action == ActionType::SUSPEND || action == ActionType::HIBERNATE;
    if (session_survives)
        close_overlays();
    else // Can't retarget a mapped surface's opacity; stack opaque overlays over the dim ones instead.
        overlays += layeroverlay::showOnAllScreens(Qt::black, LayerShellQt::Window::LayerTop, "forest-logout-dim");

    // Stay alive past the compositor's fade (~220ms) so the fade-out can finish.
    QTimer::singleShot(250, this, [this, action, session_survives](){
        const ActionData action_data = action_map.value(action);
        settings->setValue("lastaction", action_data.key);
        settings->sync();

        const QDBusError error = call_login1(action_data.method);
        if (error.isValid()){
            qWarning() << "login1" << action_data.method << "failed:" << error.message();
            close_overlays();
            QMessageBox::warning(nullptr, "Logout", QString("Failed to %1: %2").arg(action_data.key, error.message()));
        }
        if (error.isValid() || session_survives)
            qApp->quit();
    });
}

void logoutmanager::close_overlays(){
    for (layeroverlay *overlay : std::as_const(overlays))
        if (overlay) overlay->close();
    overlays.clear();
}

void logoutmanager::cancel(){
    close();
    close_overlays();
    QTimer::singleShot(250, qApp, SLOT(quit()));
}
