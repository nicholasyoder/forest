/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2021 Nicholas Yoder
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "logout.h"

#define HAL_SERVICE "org.freedesktop.Hal"
#define HAL_PATH "/org/freedesktop/Hal/devices/computer"
#define HAL_INTERFACE "org.freedesktop.Hal.Device.SystemPowerManagement"

#define UPOWER_SERVICE "org.freedesktop.UPower"
#define UPOWER_PATH "/org/freedesktop/UPower"
#define UPOWER_INTERFACE UPOWER_SERVICE

#define SYSTEMD_SERVICE "org.freedesktop.login1"
#define SYSTEMD_PATH "/org/freedesktop/login1"
#define SYSTEMD_INTERFACE "org.freedesktop.login1.Manager"

#define Management_SERVICE "org.freedesktop.PowerManagement"
#define Management_PATH "/org/freedesktop/PowerManagement"
#define Management_INTERFACE Management_SERVICE

#define CONSOLEKIT_SERVICE "org.freedesktop.ConsoleKit"
#define CONSOLEKIT_PATH "/org/freedesktop/ConsoleKit/Manager"
#define CONSOLEKIT_INTERFACE "org.freedesktop.ConsoleKit.Manager"

#define PROPERTIES_INTERFACE "org.freedesktop.DBus.Properties"

logoutmanager::logoutmanager(){
    setWindowFlags(Qt::X11BypassWindowManagerHint);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_TranslucentBackground);

    setup();

    if (QX11Info::isCompositingManagerRunning())
        backwidget->show();
}

logoutmanager::~logoutmanager(){

}

void logoutmanager::startbackfade(){
    if(starttimer->isActive())
        starttimer->stop();
    if(fallbacktimer->isActive())
        fallbacktimer->stop();

    if (QX11Info::isCompositingManagerRunning()){
        backwidget->start();
        QTimer::singleShot(200, this, SLOT(showwindow()));
    }
    else{
        this->setWindowOpacity(1.0);
        QTimer::singleShot(100, this, SLOT(activate()));
        focusbt->setFocus();
    }
}

void logoutmanager::showwindow(){
    connect(fadetimer, SIGNAL(timeout()), this, SLOT(fadein()));
    fadetimer->start(30);
}

void logoutmanager::fadein(){
    if (opacity < 1.0){
        opacity = opacity + 0.25;
        setWindowOpacity(opacity);
    }
    else{
        fadetimer->stop();
        activateWindow();
        focusbt->setFocus();
    }
}

void logoutmanager::fadeout()
{
    if (opacity > 0.0){
        opacity = opacity - 0.25;
        setWindowOpacity(opacity);
    }
    else{
        fadetimer->stop();
        close();
    }
}

void logoutmanager::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Left)
        this->focusPreviousChild();
    else if (event->key() == Qt::Key_Right)
        this->focusNextChild();
    else if (event->key() == Qt::Key_Escape)
        cancel();
}

void logoutmanager::setup()
{
    QVBoxLayout *basevlayout = new QVBoxLayout;
    basevlayout->setSpacing(0);
    basevlayout->setMargin(0);

    QHBoxLayout *closehlayout = new QHBoxLayout;
    closehlayout->setMargin(0);
    QLabel *titlelabel = new QLabel("Choose Action:");
    titlelabel->setObjectName("logout_TitleLabel");
    closehlayout->addWidget(titlelabel);
    closehlayout->addStretch(5);

    iconbutton *closebt = new iconbutton(QIcon::fromTheme("dialog-close"), 16, 16);
    connect(closebt, SIGNAL(clicked()), this, SLOT(cancel()));
    closehlayout->addWidget(closebt);
    basevlayout->addLayout(closehlayout);

    QSize btsize(85,85);
    QHBoxLayout *actionshlayout = new QHBoxLayout;
    actionshlayout->setSpacing(0);
    iconbutton *shutdownbt = new iconbutton(QIcon::fromTheme("system-shutdown"), 48, 48, true, "Shutdown");
    shutdownbt->setFixedSize(btsize);
    actionshlayout->addWidget(shutdownbt);
    iconbutton *rebootbt = new iconbutton(QIcon::fromTheme("system-reboot"), 48, 48, true, "Reboot");
    rebootbt->setFixedSize(btsize);
    actionshlayout->addWidget(rebootbt);
    iconbutton *logoutbt = new iconbutton(QIcon::fromTheme("system-log-out"), 48, 48, true, "Logout");
    logoutbt->setFixedSize(btsize);
    actionshlayout->addWidget(logoutbt);
    iconbutton *suspendbt = new iconbutton(QIcon::fromTheme("system-suspend"), 48, 48, true, "Suspend");
    suspendbt->setFixedSize(btsize);
    actionshlayout->addWidget(suspendbt);
    iconbutton *hibernatebt = new iconbutton(QIcon::fromTheme("system-suspend-hibernate"), 48, 48, true, "Hibernate");
    hibernatebt->setFixedSize(btsize);
    actionshlayout->addWidget(hibernatebt);
    basevlayout->addLayout(actionshlayout);

    QString lastaction = settings->value("lastaction", "shutdown").toString();
    if (lastaction == "shutdown") focusbt = shutdownbt;
    else if (lastaction == "reboot") focusbt = rebootbt;
    else if (lastaction == "logout") focusbt = logoutbt;
    else if (lastaction == "suspend") focusbt = suspendbt;
    else if (lastaction == "hibernate") focusbt = hibernatebt;

    if (QX11Info::isCompositingManagerRunning()){
        connect(shutdownbt, SIGNAL(clicked()), this, SLOT(startshutdown()));
        connect(rebootbt, SIGNAL(clicked()), this, SLOT(startreboot()));
        connect(logoutbt, SIGNAL(clicked()), this, SLOT(startlogout()));
        connect(suspendbt, SIGNAL(clicked()), this, SLOT(startsuspend()));
        connect(hibernatebt, SIGNAL(clicked()), this, SLOT(starthibernate()));
    }
    else{
        connect(shutdownbt, SIGNAL(clicked()), this, SLOT(shutdown()));
        connect(rebootbt, SIGNAL(clicked()), this, SLOT(reboot()));
        connect(logoutbt, SIGNAL(clicked()), this, SLOT(logout()));
        connect(suspendbt, SIGNAL(clicked()), this, SLOT(suspend()));
        connect(hibernatebt, SIGNAL(clicked()), this, SLOT(hibernate()));
    }

    QFrame *mainframe = new QFrame;
    mainframe->setObjectName("logout_MainWindow");
    mainframe->setLayout(basevlayout);
    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setMargin(0);
    vlayout->addWidget(mainframe);

    QWidget::setTabOrder(shutdownbt, rebootbt);
    QWidget::setTabOrder(rebootbt, logoutbt);
    QWidget::setTabOrder(logoutbt, suspendbt);
    QWidget::setTabOrder(suspendbt, hibernatebt);
    QWidget::setTabOrder(hibernatebt, closebt);
}

void logoutmanager::startshutdown()
{
    settings->setValue("lastaction", "shutdown");
    delete fadetimer;
    fadetimer = new QTimer;
    connect(fadetimer, SIGNAL(timeout()), this, SLOT(fadeout()));
    fadetimer->start(30);
    backwidget->blackout();
    QTimer::singleShot(1000, this, SLOT(shutdown()));
}

void logoutmanager::startreboot()
{
    settings->setValue("lastaction", "reboot");
    delete fadetimer;
    fadetimer = new QTimer;
    connect(fadetimer, SIGNAL(timeout()), this, SLOT(fadeout()));
    fadetimer->start(30);
    backwidget->blackout();
    QTimer::singleShot(1000, this, SLOT(reboot()));
}

void logoutmanager::startlogout()
{
    settings->setValue("lastaction", "logout");
    delete fadetimer;
    fadetimer = new QTimer;
    connect(fadetimer, SIGNAL(timeout()), this, SLOT(fadeout()));
    fadetimer->start(30);
    backwidget->blackout();
    QTimer::singleShot(1000, this, SLOT(logout()));
}

void logoutmanager::startsuspend()
{
    settings->setValue("lastaction", "suspend");
    delete fadetimer;
    fadetimer = new QTimer;
    connect(fadetimer, SIGNAL(timeout()), this, SLOT(fadeout()));
    fadetimer->start(30);
    backwidget->blackout();
    QTimer::singleShot(1000, this, SLOT(suspend()));
}

void logoutmanager::starthibernate()
{
    settings->setValue("lastaction", "hibernate");
    delete fadetimer;
    fadetimer = new QTimer;
    connect(fadetimer, SIGNAL(timeout()), this, SLOT(fadeout()));
    fadetimer->start(30);
    backwidget->blackout();
    QTimer::singleShot(1000, this, SLOT(hibernate()));
}


void logoutmanager::cancel()
{
    delete fadetimer;
    fadetimer = new QTimer;
    connect(fadetimer, SIGNAL(timeout()), this, SLOT(fadeout()));
    fadetimer->start(30);

    backwidget->stop();

    QTimer::singleShot(1000, qApp, SLOT(quit()));
}

void logoutmanager::shutdown()
{
    if(dbusCall(CONSOLEKIT_SERVICE,CONSOLEKIT_PATH ,CONSOLEKIT_INTERFACE , "Stop" ))
    {
        //this->close();
    }
    else
    {
        if( dbusCall(SYSTEMD_SERVICE,SYSTEMD_PATH ,SYSTEMD_INTERFACE, "PowerOff" ,true ))
        {
            //this->close();
        }
        else
        {
            QMessageBox *box = new QMessageBox;
            box->setText("Can not Shutdown.");
            box->show();
        }
    }
    qApp->quit();
}

void logoutmanager::reboot()
{
    if(dbusCall(CONSOLEKIT_SERVICE,CONSOLEKIT_PATH ,CONSOLEKIT_INTERFACE , "Restart" ))
    {
        //this->close();
    }
    else
    {
        if( dbusCall(SYSTEMD_SERVICE,SYSTEMD_PATH ,SYSTEMD_INTERFACE ,  "Reboot" ,true ))
        {
            //this->close();
        }
        else
        {
            QMessageBox *box = new QMessageBox;
            box->setText("Can not Reboot.");
            box->show();
        }
    }
    qApp->quit();
}

void logoutmanager::logout()
{

    /*QProcess *p = new QProcess;
    p->setWorkingDirectory(QDir::homePath());
    p->start("env");
    p->waitForFinished();
    QString output = p->readAllStandardOutput();

    qDebug(qPrintable(output));
    if (output.contains("XDG_SESSION_ID"))
    {
        QStringList entrys = output.split("\n");
        QStringList entry = entrys.filter("XDG_SESSION_ID");
        QStringList listid = entry.first().split("=");
        QString id = listid.last();
        qDebug(qPrintable(id));
    }*/

    QDBusInterface iface("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", QDBusConnection::systemBus());
    iface.call("TerminateSession", "");
    /*
    if (QDBusConnection::sessionBus().isConnected())
    {
        QDBusInterface iface("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", QDBusConnection::systemBus());
        if (iface.isValid())
        {
            iface.call("TerminateSession", "");
        }
        else
        {
            fprintf(stderr, "%s\n", qPrintable(QDBusConnection::sessionBus().lastError().message()));
        }
    }
    else
    {
        fprintf(stderr, "Cannot connect to the D-Bus session bus.\n"
                "To start it, run:\n"
                "\teval `dbus-launch --auto-syntax`\n");
    }*/

    /*if( dbusCall(SYSTEMD_SERVICE,SYSTEMD_PATH ,SYSTEMD_INTERFACE, "TerminateSession" ,true ))
    {
        //this->close();
    }
    else
    {
        QMessageBox *box = new QMessageBox;
        box->setText("Can not Logout.");
        box->show();
    }*/
    //qApp->quit();

    /*if (QDBusConnection::sessionBus().isConnected())
    {
        QDBusInterface iface("com.forest.session.manager", "/manager", "", QDBusConnection::sessionBus());
        if (iface.isValid())
        {
            iface.call("logout");
        }
        else
        {
            fprintf(stderr, "%s\n", qPrintable(QDBusConnection::sessionBus().lastError().message()));
        }
    }
    else
    {
        fprintf(stderr, "Cannot connect to the D-Bus session bus.\n"
                "To start it, run:\n"
                "\teval `dbus-launch --auto-syntax`\n");
    }*/
}

void logoutmanager::suspend()
{
    if(dbusCall(Management_SERVICE,Management_PATH ,Management_INTERFACE ,"Suspend"  ))
    {

    }
    else
    {
        if( dbusCall(SYSTEMD_SERVICE,SYSTEMD_PATH ,SYSTEMD_INTERFACE ,  "Suspend" ,true ))
        {

        }
        else
        {
            if (dbusCall(UPOWER_SERVICE,UPOWER_PATH ,UPOWER_INTERFACE,"Suspend"))
            {

            }
            else
            {
                QMessageBox *box = new QMessageBox;
                box->setText("Can not Suspend.");
                box->show();
            }
        }
    }
    qApp->quit();
}

void logoutmanager::hibernate()
{
    if(dbusCall(Management_SERVICE,Management_PATH ,Management_INTERFACE ,"Hibernate"  ))
    {

    }
    else
    {
        if( dbusCall(SYSTEMD_SERVICE,SYSTEMD_PATH ,SYSTEMD_INTERFACE ,  "Hibernate" ,true ))
        {

        }
        else
        {
            if (dbusCall(UPOWER_SERVICE,UPOWER_PATH ,UPOWER_INTERFACE,"Hibernate"))
            {

            }
            else
            {
                QMessageBox *box = new QMessageBox;
                box->setText("Can not Hibernate.");
                box->show();
            }
        }
    }
    qApp->quit();
}

bool logoutmanager::dbusCall(const QString &service,const QString &path, const QString &interface,const QString & method, bool sysd)
{
    QDBusInterface dbus(service, path, interface, QDBusConnection::systemBus());
    if (!dbus.isValid())
    {
        qWarning() << "dbusCall: QDBusInterface is invalid" << service <<  method;
        return false;
    }

    QDBusMessage msg ;
    if(sysd)
    {
        msg = dbus.call(method,true);
        if (msg.arguments().isEmpty() || msg.arguments().first().isNull())
            return true;

        QString response = msg.arguments().first().toString();
        qDebug() << "systemd:" << method << "=" << response;
        return response == "yes" || response == "challenge";

    }else
        msg = dbus.call(method);

    return msg.arguments().isEmpty() ||
            msg.arguments().first().isNull() ||
            msg.arguments().first().toBool();
}
