#include "sessionapp.h"

#include <QProcess>
#include <QDebug>

#include <qt5xdg/XdgAutoStart>

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
    if(cmd.contains(" "))
        QProcess::startDetached("/bin/bash", QStringList() << "-c" << cmd);
    else
        QProcess::startDetached(cmd, QStringList());
}
