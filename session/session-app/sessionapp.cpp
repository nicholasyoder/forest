// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sessionapp.h"

#include <QProcess>
#include <QDebug>
#include <QTimer>

#include <iterator>

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
    // Setup Keyboard

    // Setup Mouse

    startProcess("forest");
    startLocker();
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

// Restarted unless it quits cleanly: displays it turned off stay dark without it.
void SessionApp::startLocker(){
    static constexpr int delaysMs[] = {0, 1000, 5000, 30000};
    if (!locker) {
        locker = new QProcess(this);
        locker->setProcessChannelMode(QProcess::ForwardedChannels);
        connect(locker, &QProcess::finished, this, [this](int code, QProcess::ExitStatus status) {
            if (status == QProcess::NormalExit && code == 0)
                return;
            lockerFastCrashes = lockerUptime.elapsed() < 30000 ? lockerFastCrashes + 1 : 0;
            int delay = delaysMs[qMin(lockerFastCrashes, int(std::size(delaysMs)) - 1)];
            qWarning() << "forest-locker exited (status" << status << "code" << code << "), restarting in" << delay << "ms";
            QTimer::singleShot(delay, this, &SessionApp::startLocker);
        });
    }
    lockerUptime.start();
    locker->start("forest-locker", QStringList());
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
