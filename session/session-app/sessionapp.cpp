// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sessionapp.h"

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

    // Setup Keyboard

    // Setup Mouse

    launch_WM();
    startProcess("forest");
    launch_autostart_commands();
    launch_autostart_xdg();
}

void SessionApp::launch_WM(){
    QString wm_cmd = settings->value("window_manager", "xfwm4").toString();
    qInfo() << "Launching window manager:" << wm_cmd;
    startProcess(wm_cmd);
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
