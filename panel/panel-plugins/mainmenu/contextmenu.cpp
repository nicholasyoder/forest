// SPDX-License-Identifier: LGPL-3.0-or-later

#include "contextmenu.h"
#include "propertieswidget.h"
#include "menuanchor.h"

#include <QMenu>
#include <QtDBus>
#include <qt6xdg/XdgIcon>

contextmenu::contextmenu()
{
    cmenu = new QMenu;
    cmenu->addAction(XdgIcon::fromTheme("list-add"), "Add to quicklaunch", this, &contextmenu::add2Quicklaunch);
    cmenu->addAction(XdgIcon::fromTheme("user-desktop"), "Show on desktop", this, &contextmenu::showOnDesktop);
    //cmenu->addAction(XdgIcon::fromTheme("application-x-executable"), "Run as root", this, &contextmenu::runAsRoot);
    cmenu->addAction(XdgIcon::fromTheme("edit-entry"), "Properties", this, &contextmenu::showProperties);
}

contextmenu::~contextmenu()
{
    delete cmenu;
}

void contextmenu::show(XdgDesktopFile deskfile, QWidget *anchor, QPoint pos){
    currentDeskFile = deskfile;
    menuanchor::anchorMenuAtPoint(cmenu, anchor, pos);
    cmenu->popup(anchor->mapToGlobal(pos));
}

void contextmenu::add2Quicklaunch(){
    if (QDBusConnection::sessionBus().isConnected()){
        QDBusInterface iface("org.forest", "/org/forest/panel/quicklaunch", "", QDBusConnection::sessionBus());
        if (iface.isValid())
            iface.call("addlauncher", currentDeskFile.fileName());
        else
            fprintf(stderr, "%s\n", qPrintable(QDBusConnection::sessionBus().lastError().message()));
    }
    else{
        fprintf(stderr, "Cannot connect to the D-Bus session bus.\nTo start it, run:\n\teval `dbus-launch --auto-syntax`\n");
    }
}

void contextmenu::showOnDesktop(){
    QFile::copy(currentDeskFile.fileName(), QDir::homePath() + "/Desktop/" + currentDeskFile.fileName().split("/").last());
}

void contextmenu::runAsRoot(){

}

void contextmenu::showProperties(){
    if (currentDeskFile.isValid()){
        propertieswidget *pwidget = new propertieswidget;
        pwidget->setWindowFlags(Qt::Dialog);
        pwidget->setdata(currentDeskFile.name(),
                         currentDeskFile.localizedValue("Exec").toString(),
                         currentDeskFile.comment(),
                         currentDeskFile.iconName(),
                         currentDeskFile.fileName());
        pwidget->show();
    }
}
