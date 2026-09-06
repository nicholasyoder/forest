// SPDX-License-Identifier: LGPL-3.0-or-later

#include "logout.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScreen>
#include <QWindow>
#include <QMessageBox>
#include <QVector>

#include <LayerShellQt/Window>

enum class DBusService {SYSTEMD, CONSOLEKIT, UPOWER, PWMANAGEMENT};

struct DBusMethod {
    DBusService service;
    QString method;

    DBusMethod(const DBusService& s, const QString& m) : service(s), method(m) {}
};

struct ActionData {
    QString key;
    QList<DBusMethod> methods;
};

const QMap<ActionType, ActionData> action_map = {
    {ActionType::SHUTDOWN, {"shutdown", {DBusMethod(DBusService::SYSTEMD, "PowerOff"), DBusMethod(DBusService::CONSOLEKIT, "Stop")}}},
    {ActionType::REBOOT, {"reboot", {DBusMethod(DBusService::SYSTEMD, "Reboot"), DBusMethod(DBusService::CONSOLEKIT, "Restart")}}},
    {ActionType::LOGOUT, {"logout", {DBusMethod(DBusService::SYSTEMD, "TerminateSession")}}},
    {ActionType::SUSPEND, {"logout", {DBusMethod(DBusService::SYSTEMD, "Suspend"), DBusMethod(DBusService::UPOWER, "Suspend"), DBusMethod(DBusService::PWMANAGEMENT, "Suspend")}}},
    {ActionType::HIBERNATE, {"hibernate", {DBusMethod(DBusService::SYSTEMD, "Hibernate"), DBusMethod(DBusService::UPOWER, "Hibernate"), DBusMethod(DBusService::PWMANAGEMENT, "Hibernate")}}}
};

bool call_dbus_method(const QString &service,const QString &path, const QString &interface, const QString & method, bool sysd){
    QDBusInterface dbus(service, path, interface, QDBusConnection::systemBus());
    if (!dbus.isValid()){
        qWarning() << "Failed to connect to dbus system bus. Unable to perform dbus call: " << service <<  method;
        return false;
    }

    QDBusMessage msg;
    if(sysd){
        if (method == "TerminateSession") msg = dbus.call(method, "");
        else msg = dbus.call(method, true);
        if (msg.arguments().isEmpty() || msg.arguments().first().isNull())
            return true;

        QString response = msg.arguments().first().toString();
        qDebug() << response;
        return response == "yes" || response == "challenge";
    }
    else {
        msg = dbus.call(method);
    }
    return msg.arguments().isEmpty() || msg.arguments().first().isNull() || msg.arguments().first().toBool();
}

void call_dbus_methods(QList<DBusMethod> methods){
    //Try to call the specified dbus methods in order, quiting as soon as a call is successful.
    bool success = false;
    foreach(DBusMethod method, methods){
        QString service; QString path; QString interface;
        switch (method.service) {
        case DBusService::SYSTEMD:
            service = "org.freedesktop.login1";
            path = "/org/freedesktop/login1";
            interface = "org.freedesktop.login1.Manager";
            break;
        case DBusService::CONSOLEKIT:
            service = "org.freedesktop.ConsoleKit";
            path = "/org/freedesktop/ConsoleKit/Manager";
            interface = "org.freedesktop.ConsoleKit.Manager";
            break;
        case DBusService::UPOWER:
            service = interface = "org.freedesktop.UPower";
            path = "/org/freedesktop/UPower";
            break;
        case DBusService::PWMANAGEMENT:
            service = interface = "org.freedesktop.PowerManagement";
            path = "/org/freedesktop/PowerManagement";
            break;
        default:
            break;
        }
        if(call_dbus_method(service, path, interface, method.method, method.service == DBusService::SYSTEMD)){
            success = true;
            break;
        }
    }
    if(!success){
        QString msg = "Failed to call dbus methods: ";
        foreach(DBusMethod method, methods)
            msg += method.method + " ";

        qWarning() << msg;
        qApp->quit();
    }
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

    QRect screen_geo = qApp->primaryScreen()->geometry();
    layer_window->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorTop | LayerShellQt::Window::AnchorLeft));
    layer_window->setMargins(QMargins(
        screen_geo.x() + (screen_geo.width() / 2 - sizeHint().width() / 2),
        screen_geo.y() + (screen_geo.height() / 2 - sizeHint().height() / 2),
        0, 0
    ));

    foreach(QScreen* screen, qApp->screens()){
        imagewidget *background_fader = new imagewidget;
        background_fader->windowHandle()->setScreen(screen);
        background_fader->setFixedSize(screen->size());
        background_fader->show();
        background_faders.append(background_fader);
    }
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
    if (lastaction == "shutdown") focusbt = shutdownbt;
    else if (lastaction == "reboot") focusbt = rebootbt;
    else if (lastaction == "logout") focusbt = logoutbt;
    else if (lastaction == "suspend") focusbt = suspendbt;
    else if (lastaction == "hibernate") focusbt = hibernatebt;
    connect(shutdownbt, &iconbutton::clicked, this, [this](){start_action(ActionType::SHUTDOWN);});
    connect(rebootbt, &iconbutton::clicked, this, [this](){start_action(ActionType::REBOOT);});
    connect(logoutbt, &iconbutton::clicked, this, [this](){start_action(ActionType::LOGOUT);});
    connect(suspendbt, &iconbutton::clicked, this, [this](){start_action(ActionType::SUSPEND);});
    connect(hibernatebt, &iconbutton::clicked, this, [this](){start_action(ActionType::HIBERNATE);});
    QWidget::setTabOrder(shutdownbt, rebootbt);
    QWidget::setTabOrder(rebootbt, logoutbt);
    QWidget::setTabOrder(logoutbt, suspendbt);
    QWidget::setTabOrder(suspendbt, hibernatebt);
    QWidget::setTabOrder(hibernatebt, closebt);
}

void logoutmanager::startbackfade(){
    // The dialog and its dim overlays already fade in on their own the
    // moment each is mapped - Biome fades any layer-shell surface whose
    // namespace is configured for it (see biome/desktop/layer_shell.cpp and
    // biome/core/fade_config.h). Two independent mechanisms/config keys:
    // the dialog's own "forest-logout" namespace uses the simple per-pixel
    // opacity fade ([LayerShell]/fadingNamespaces); the dim overlay's
    // "forest-logout-dim" namespace (imagewidget.cpp) uses the opaque
    // scanout-snapshot fade ([LayerShell]/scanoutFadingNamespaces), which
    // avoids the composited-render-path cost a fullscreen translucent
    // overlay would otherwise force on every tick. Nothing left to do here
    // but grab focus.
    set_initial_focus();
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
    close(); // fades out via Biome (see startbackfade())

    // Rather than retargeting the existing dim overlays' opacity (Wayland
    // has no protocol for a client to retarget an already-mapped surface's
    // opacity), open new fully-opaque ones - they fade in from whatever's
    // behind them (the old half-dim overlays, deliberately left open to
    // avoid any flicker gap) up to full black. The old overlays are never
    // explicitly closed; the process exits shortly after regardless.
    foreach(QScreen* screen, qApp->screens()){
        imagewidget *blackout_widget = new imagewidget(imagewidget::DimLevel::Full);
        blackout_widget->windowHandle()->setScreen(screen);
        blackout_widget->setFixedSize(screen->size());
        blackout_widget->show();
    }

    // Comfortably above Biome's own kFadeDurationMs (desktop/layer_shell.cpp,
    // 220ms) - the process staying alive/connected for this long is what
    // lets the dialog's own opacity fade-out actually finish before its
    // content goes away underneath it.
    QTimer::singleShot(250, this, [this, action](){do_action(action);});
}

void logoutmanager::do_action(ActionType action){
    auto action_data = action_map.value(action);
    settings->setValue("lastaction", action_data.key);
    settings->sync();
    call_dbus_methods(action_data.methods);
}

void logoutmanager::cancel(){
    close(); // fades out via Biome
    foreach(imagewidget* background_fader, background_faders)
        background_fader->close(); // fades out via Biome
    // See start_action()'s matching comment above.
    QTimer::singleShot(250, qApp, SLOT(quit()));
}
