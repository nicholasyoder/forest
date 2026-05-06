// SPDX-License-Identifier: LGPL-3.0-or-later

#include "logout.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScreen>
#include <QMessageBox>
#include <QVector>

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
    setWindowFlags(Qt::X11BypassWindowManagerHint);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowOpacity(0.0);
    settings = new QSettings("Forest", "Logout");
    setup();

    foreach(QScreen* screen, qApp->screens()){
        imagewidget *background_fader = new imagewidget;
        background_fader->setGeometry(screen->geometry());
        background_fader->show();
        background_faders.append(background_fader);
    }

    QRect screen_geo = qApp->primaryScreen()->geometry();
    move(
        screen_geo.x() + (screen_geo.width() / 2 - sizeHint().width() / 2),
        screen_geo.y() + (screen_geo.height() / 2 - sizeHint().height() / 2)
    );
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
    foreach(imagewidget* background_fader, background_faders)
        background_fader->start();

    do_fade(FadeDirection::FADEIN);
}

void logoutmanager::set_initial_focus(){
    activateWindow();
    focusbt->setFocus();
}

void logoutmanager::do_fade(FadeDirection direction, int interval, float limit){
    fade_timer = new QTimer(this);
    connect(fade_timer, &QTimer::timeout, this, [this, direction, limit](){perform_fade_step(direction, limit);});
    fade_timer->start(interval);
}

void logoutmanager::perform_fade_step(FadeDirection direction, float limit){
    if(direction == FadeDirection::FADEIN){
        if (fade_opacity < 1.0 - limit){
            fade_opacity += 0.2;
            setWindowOpacity(fade_opacity);
            update();
        }
        else{
            fade_timer->stop();
            set_initial_focus();
        }
    }
    else{
        if (fade_opacity > 0.0 + limit){
            fade_opacity -= 0.2;
            setWindowOpacity(fade_opacity);
            update();
        }
        else{
            fade_timer->stop();
            close();
        }
    }
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
    do_fade(FadeDirection::FADEOUT);
    foreach(imagewidget* background_fader, background_faders)
        background_fader->blackout();

    QTimer::singleShot(1000, this, [this, action](){do_action(action);});
}

void logoutmanager::do_action(ActionType action){
    auto action_data = action_map.value(action);
    settings->setValue("lastaction", action_data.key);
    settings->sync();
    call_dbus_methods(action_data.methods);
}

void logoutmanager::cancel(){
    do_fade(FadeDirection::FADEOUT);
    foreach(imagewidget* background_fader, background_faders)
        background_fader->stop();
    QTimer::singleShot(1000, qApp, SLOT(quit()));
}
