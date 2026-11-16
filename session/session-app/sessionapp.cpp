// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sessionapp.h"
#include "supervisedprocess.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QProcess>
#include <QDebug>

#include <qt6xdg/XdgAutoStart>

SessionApp::SessionApp(){
    settings = new QSettings("Forest", "Session");
}

SessionApp::~SessionApp(){

}

void SessionApp::startSession(){

    // Setup environment
    // startforest-wayland's update runs before Biome creates the socket; push it before anything D-Bus-activates.
    if (QProcess::execute("dbus-update-activation-environment", {"--systemd", "WAYLAND_DISPLAY"}) != 0)
        qWarning() << "Failed to export WAYLAND_DISPLAY to the activation environment";
    // The PAM-started keyring daemon exits after a few idle minutes; activating
    // org.freedesktop.secrets now makes a daemon attach to it and keep it unlocked.
    QDBusConnection::sessionBus().asyncCall(QDBusMessage::createMethodCall(
        "org.freedesktop.secrets", "/org/freedesktop/secrets", "org.freedesktop.DBus.Peer", "Ping"));

    // Setup Keyboard

    // Setup Mouse

    // Displays it turned off stay dark without the locker; the shell is nothing without forest.
    (new SupervisedProcess("forest", 1000, this))->start();
    (new SupervisedProcess("forest-locker", 0, this))->start();
    launch_autostart_commands();
    launch_autostart_xdg();
}

void SessionApp::launch_autostart_commands(){
    settings->beginGroup("Autostart");
    QStringList items = settings->childGroups();
    foreach(QString item, items){
        settings->beginGroup(item);
        QString command = settings->value("command").toString();
        startProcess(command);
        settings->endGroup();
    }
    settings->endGroup();
}

void SessionApp::launch_autostart_xdg(){
    if (!settings->value("launch_xdg_autostart", true).toBool())
        return;
    XdgDesktopFileList fileList = XdgAutoStart::desktopFileList();
    foreach (XdgDesktopFile xdgfile, fileList)
        xdgfile.startDetached();
}

void SessionApp::startProcess(QString cmd){
    bool ok;
    if(cmd.contains(" "))
        ok = QProcess::startDetached("/bin/bash", QStringList() << "-c" << cmd);
    else
        ok = QProcess::startDetached(cmd, QStringList());
    if (!ok)
        qWarning() << "Failed to start process:" << cmd;
}
